#include <chrono>
#include <list>
#include <utility>
#include <vector>

#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <nlohmann/json.hpp>

#include "3dprnet/core/error.hpp"
#include "3dprnet/core/logging.hpp"
#include "3dprnet/repetier/client.hpp"
#include "3dprnet/repetier/service.hpp"
#include "3dprnet/repetier/types.hpp"
#include "3dprnet/repetier/upload.hpp"

using namespace std;
using namespace nlohmann;

namespace asio = boost::asio;

namespace prnet {
namespace rep {

static Logger logger( "rep::Service" );
    
namespace detail {
    
inline long retryTimeout( size_t retry ) 
{
    switch ( retry ) {
        case 0: return 0;
        case 1: return 2;
        case 2: return 5;
        case 3: return 10;
        default: return 30;
    }
}

inline void checkResponseOk( json const& data )
{
    if ( !data.at( "ok" ) ) {
        throw system_error( make_error_code( prnet_errc::not_ok ) );
    }
}

inline json makeRequest( string action )
{
    return {
        { "action", move( action ) },
        { "data", json::object() }
    };
}

inline json makeRequest( string action, string slug )
{
    auto request = makeRequest( move( action ) );
    request.emplace( "printer", move( slug ) );
    return move( request );
}

} // namespace detail


/**
 * class Service
 */

struct Service::Action
{
    Action( json&& request, CallbackHandler&& handler )
            : request( move( request ) )
            , handler( move( handler ) ) {}

    json request;
    CallbackHandler handler;
};
    
class Service::ServiceImpl
{
public:
    ServiceImpl( boost::asio::io_context& context, Endpoint&& endpoint )
            : context_( context )
            , endpoint_( move( endpoint ) )
    {
        connect();
    }

    bool connected() const { return connected_; }

    void request_printers()
    {
        if ( on_printers_.empty() ) {
            return;
        }

        send( detail::makeRequest( "listPrinter" ), [this]( auto const& data ) {
            on_printers_( data );
        } );
    }

    void request_config( string&& slug )
    {
        if ( on_config_.empty() ) {
            return;
        }

        auto request = detail::makeRequest( "getPrinterConfig", slug );
        send( move( request ), [this, slug = move( slug )]( auto const& data ) mutable {
            on_config_( move( slug ), data );
        } );
    }

    void request_groups( string&& slug )
    {
        if ( on_groups_.empty() ) {
            return;
        }

        auto request = detail::makeRequest( "listModelGroups", slug );
        send( move( request ), [this, slug = move( slug )]( auto const& data ) mutable {
            detail::checkResponseOk( data );
            on_groups_( move( slug ), data.at( "groupNames" ) );
        } );
    }

    void request_models( string&& slug )
    {
        if ( on_models_.empty() ) {
            return;
        }

        auto request = detail::makeRequest( "listModels", slug );
        send( move( request ), [this, slug = move( slug )]( auto const& data ) mutable {
            on_models_( move( slug ), data.at( "data" ) );
        } );
    }

    void addModelGroup( string &&slug, string &&group, Handler &&handler )
    {
        auto request = detail::makeRequest( "addModelGroup", move( slug ) );
        request[ "data" ].emplace( "groupName", move( group ) );
        send( move( request ), [this, handler = move( handler )]( auto const& data ) {
            detail::checkResponseOk( data );
            handler();
        } );
    }

    void deleteModelGroup( string &&slug, string &&group, bool deleteModels, Handler &&handler )
    {
        auto request = detail::makeRequest( "delModelGroup", move( slug ) );
        request[ "data" ].emplace( "groupName", move( group ) );
        request[ "data" ].emplace( "delFiles", deleteModels );
        send( move( request ), [this, handler = move( handler )]( auto const& data ) {
            detail::checkResponseOk( data );
            handler();
        } );
    }

    void removeModel( string &&slug, size_t id, Handler &&handler )
    {
        auto request = detail::makeRequest( "removeModel", move( slug ) );
        request[ "data" ].emplace( "id", id );
        send( move( request ), [this, handler = move( handler )]( auto const& ) {
            handler();
        } );
    }

    void moveModelToGroup( string &&slug, size_t id, string &&group, Handler &&handler )
    {
        auto request = detail::makeRequest( "moveModelFileToGroup", move( slug ) );
        request[ "data" ].emplace( "id", id );
        request[ "data" ].emplace( "groupName", move( group ) );
        send( move( request ), [this, handler = move( handler )]( auto const& data ) {
            detail::checkResponseOk( data );
            handler();
        } );
    }

    void sendCommand( string&& slug, string&& command, Handler&& handler )
    {
        auto request = detail::makeRequest( "send", move( slug ) );
        request[ "data" ].emplace( "cmd", move( command ) );
        send( move( request ), [this, handler = move( handler )]( auto const& data ) {
            handler();
        } );
    }

    void upload( model_ident&& ident, filesystem::path&& path, UploadHandler&& handler )
    {
        uploadModel( context_, endpoint_, move( ident ), move( path ), [this, handler = move( handler )]( auto ec ) {
            handler( ec );
        } );
    }


    void on_reconnect( ReconnectEvent::slot_type const& handler )
    {
        on_reconnect_.connect( handler );
    }

    void on_disconnect( DisconnectEvent::slot_type const& handler )
    {
        on_disconnect_.connect( handler );
    }

    void on_temperature( TemperatureEvent::slot_type const& handler )
    {
        on_temperature_.connect( handler );
    }

    void on_printers( PrintersEvent::slot_type const& handler )
    {
        on_printers_.connect( handler );
    }

    void on_config( ConfigEvent::slot_type const& handler )
    {
        on_config_.connect( handler );
    }

    void on_groups( GroupsEvent::slot_type const& handler )
    {
        on_groups_.connect( handler );
    }

    void on_models( ModelsEvent::slot_type const& handler )
    {
        on_models_.connect( handler );
    }

private:
    void connect()
    {
        logger.info( "initiating connection to server" );

        client_ = make_unique< Client >( context_, [this]( auto ec ) { this->handle_error( ec ); } );
        client_->subscribe( "temp", [this]( auto slug, auto data ) { on_temperature_( move( slug ), move( data ) ); } );
        client_->subscribe( "printerListChanged", [this]( auto, auto data ) { on_printers_( move( data ) ); } );
        client_->subscribe( "config", [this]( auto slug, auto data ) { on_config_( move( slug ), move( data ) ); } );
        client_->subscribe( "modelGroupListChanged", [this]( auto slug, auto ) { this->request_groups( move( slug ) ); } );
        client_->subscribe( "jobsChanged", [this]( auto slug, auto ) { this->request_models( move( slug ) ); } );
        client_->subscribe( "jobFinished", [this]( auto slug, auto ) { this->request_printers(); } );
        client_->connect( endpoint_, [this] { this->handle_connected(); } );
    }

    void send( json&& request, CallbackHandler handler, bool priority = false )
    {
        queued_.emplace( priority ? queued_.begin() : queued_.end(), move( request ), move( handler ) );
        send_next( priority );
    }

    void send_next( bool force = false )
    {
        if ( ( connected_ || force ) && !pending_ && !queued_.empty() ) {
            auto& action = queued_.front();
            client_->send( action.request, [this, &action]( auto const& data ) {
                action.handler( data );
                this->handle_sent();
            } );
            pending_ = true;
        }
    }

    void handle_connected()
    {
        logger.debug( "sending login request" );

        auto request = detail::makeRequest( "login" );
        request[ "data" ].emplace( "apikey", endpoint_.apikey() );
        send( move( request ), [this]( auto const& data ) {
            detail::checkResponseOk( data );
            this->handle_login();
        }, true );
    }

    void handle_login()
    {
        logger.info( "successfully connected and logged in" );

        connected_ = true;
        retry_ = 0;
        on_reconnect_();
    }

    void handle_sent()
    {
        queued_.pop_front();
        pending_ = false;
        send_next();
    }

    void handle_error( error_code ec )
    {
        connected_ = false;
        client_ = nullptr;
        pending_ = false;

        on_disconnect_( ec );

        long timeout = detail::retryTimeout( retry_++ );

        logger.error( "error in server communication, reconnecting in ", timeout, " seconds: ", ec.message() );

        auto timer = make_shared< asio::steady_timer >( context_, chrono::seconds( timeout ) );
        timer->async_wait( [this, timer]( auto ) { this->connect(); } );
    }

    asio::io_context& context_;
    Endpoint endpoint_;
    unique_ptr< Client > client_;
    bool connected_ {};
    bool pending_ {};
    size_t retry_ {};
    list< Action > queued_;

    ReconnectEvent on_reconnect_;
    DisconnectEvent on_disconnect_;
    TemperatureEvent on_temperature_;
    PrintersEvent on_printers_;
    ConfigEvent on_config_;
    GroupsEvent on_groups_;
    ModelsEvent on_models_;
};

Service::Service( asio::io_context &context, Endpoint endpoint )
        : impl_( make_unique< ServiceImpl >( context, move( endpoint ) ) ) {}

Service::~Service() = default;

bool Service::connected() const { return impl_->connected(); }

void Service::request_printers()
{
    impl_->request_printers();
}
    
void Service::request_config( string slug )
{
    impl_->request_config( move( slug ) );
}
    
void Service::request_groups( string slug )
{
    impl_->request_groups( move( slug ) );
}

void Service::request_models( string slug )
{
    impl_->request_models( move( slug ) );
}

void Service::upload( model_ident ident, filesystem::path path, UploadHandler handler )
{
    impl_->upload( move( ident ), move( path ), move( handler ) );
}

void Service::addModelGroup( string slug, string group, Handler handler )
{
    impl_->addModelGroup( move( slug ), move( group ), move( handler ));
}

void Service::deleteModelGroup( string slug, string group, bool deleteModels, Handler handler )
{
    impl_->deleteModelGroup( move( slug ), move( group ), deleteModels, move( handler ));
}

void Service::removeModel( string slug, size_t id, Handler handler )
{
    impl_->removeModel( move( slug ), id, move( handler ));
}

void Service::moveModelToGroup( string slug, size_t id, string group, Handler handler )
{
    impl_->moveModelToGroup( move( slug ), id, move( group ), move( handler ));
}

void Service::sendCommand( string slug, string command, Handler handler )
{
    impl_->sendCommand( move( slug ), move( command ), move( handler ) );
}

void Service::on_reconnect( ReconnectEvent::slot_type const& handler )
{
    impl_->on_reconnect( handler );
}

void Service::on_disconnect( DisconnectEvent::slot_type const& handler )
{
    impl_->on_disconnect( handler );
}

void Service::on_temperature( TemperatureEvent::slot_type const& handler )
{
    impl_->on_temperature( handler );
}

void Service::on_printers( PrintersEvent::slot_type const& handler )
{
    impl_->on_printers( handler );
}

void Service::on_config( ConfigEvent::slot_type const& handler )
{
    impl_->on_config( handler );
}

void Service::on_groups( GroupsEvent::slot_type const& handler )
{
    impl_->on_groups( handler );
}

void Service::on_models( ModelsEvent::slot_type const& handler )
{
    impl_->on_models( handler );
}

} // namespace rep
} // namespace prnet
