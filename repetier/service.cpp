#include <chrono>
#include <utility>

#include <boost/asio/steady_timer.hpp>
#include <nlohmann/json.hpp>

#include "core/error.hpp"
#include "core/logging.hpp"
#include "service.hpp"
#include "client.hpp"

using namespace std;
using namespace nlohmann;

namespace asio = boost::asio;

namespace prnet {
namespace rep {

static logger logger( "rep::Service" );
    
namespace detail {
    
inline long retryTimeout( size_t retry ) 
{
    switch ( retry ) {
        case 0: return 0;
        case 1: return 2;
        default: return 5;
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

Service::Service( asio::io_context &context, Endpoint endpoint )
        : context_( context )
        , endpoint_( move( endpoint ) )
{
    connect();
}

Service::~Service() = default;

void Service::request_printers()
{
	if ( on_printers_.empty() ) {
		return;
	}

    send( detail::makeRequest( "listPrinter" ), [this]( auto const& data ) {
        on_printers_( data );
    } );
}

void Service::request_config( string slug )
{
	if ( on_config_.empty() ) {
		return;
	}

    auto request = detail::makeRequest( "getPrinterConfig", slug );
    send( move( request ), [this, slug = move( slug )]( auto const& data ) mutable {
        on_config_( move( slug ), data );
    } );
}

void Service::request_groups( string slug )
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

void Service::request_models( string slug )
{
	if ( on_models_.empty() ) {
		return;
	}

    auto request = detail::makeRequest( "listModels", slug );
    send( move( request ), [this, slug = move( slug )]( auto const& data ) mutable {
        on_models_( move( slug ), data.at( "data" ) );
    } );
}

void Service::on_reconnect( reconnect_event::slot_type const& handler )
{
    on_reconnect_.connect( handler );
}

void Service::on_disconnect( disconnect_event::slot_type const& handler )
{
    on_disconnect_.connect( handler );
}

void Service::on_temperature( temperature_event::slot_type const& handler )
{
    on_temperature_.connect( handler );
}

void Service::on_printers( printers_event::slot_type const& handler )
{
    on_printers_.connect( handler );
}

void Service::on_config( config_event::slot_type const& handler )
{
    on_config_.connect( handler );
}

void Service::on_groups( groups_event::slot_type const& handler )
{
    on_groups_.connect( handler );
}

void Service::on_models( models_event::slot_type const& handler )
{
    on_models_.connect( handler );
}

void Service::connect()
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

void Service::send( json&& request, CallbackHandler handler, bool priority )
{
    auto handlerWithCleanup = [this, handler = move( handler )]( auto const& data ) {
        handler( data );
        this->handle_sent();
    };

    logger.debug( "queueing request with priority ", priority );

    queued_.emplace( priority ? queued_.begin() : queued_.end(), move( request ), move( handlerWithCleanup ) );
    if ( queued_.size() == 1 ) {
        send_next();
    }
}

void Service::send_next()
{
    logger.debug( "sending next request, connected=", connected_, ", queue=", queued_.size() );
    if ( connected_ && !queued_.empty() ) {
        auto& action = queued_.front();
        client_->send( action.request, action.handler );
    }
}

void Service::handle_connected()
{
    logger.debug( "sending login request" );
    auto request = detail::makeRequest( "login" );
    request[ "data" ].emplace( "apikey", endpoint_.apikey() );
    send( move( request ), [this]( auto const& data ) {
        detail::checkResponseOk( data );
        this->handle_login();
    }, true );
}

void Service::handle_login()
{
    logger.info( "successfully connected and logged in" );

    connected_ = true;
    retry_ = 0;
    on_reconnect_();
}

void Service::handle_sent()
{
	queued_.pop_front();
	send_next();
}

void Service::handle_error( error_code ec )
{
    connected_ = false;
    client_ = nullptr;

    on_disconnect_( ec );

    long timeout = detail::retryTimeout( retry_++ );

    logger.error( "error in server communication, reconnecting in ", timeout, " seconds: ", ec.message() );

    auto timer = make_shared< asio::steady_timer >( context_, chrono::seconds( timeout ) );
    timer->async_wait( [this, timer]( auto ) { this->connect(); } );
}

} // namespace rep
} // namespace prnet
