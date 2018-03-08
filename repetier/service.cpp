#include <chrono>
#include <utility>

#include <boost/asio/deadline_timer.hpp>
#include <core/error.hpp>

#include "core/logging.hpp"
#include "service.hpp"
#include "request.hpp"
#include "client.hpp"

using namespace std;

namespace asio = boost::asio;

namespace prnet {
namespace rep {

static logger logger( "rep::service" );
    
namespace detail {
    
inline long retryTimeout( size_t retry ) 
{
    switch ( retry++ ) {
        case 0: return 0;
        case 1: return 2;
        default: return 5;
    }
}    
    
} // namespace detail

service::service( asio::io_context &context, settings settings )
        : context_( context )
        , settings_( move( settings ) )
{
    connect();
}

service::~service() = default;

void service::request_printers()
{
    request req( *client_, "listPrinter" );
    req.add_handler( [this]( auto& data ) { on_printers_( move( data ) ); } );
    send( move( req ) );
}

void service::request_groups( string slug )
{
    request request( *client_, "listModelGroups" );
    request.printer( slug );
    request.add_handler( request::check_ok_flag() );
    request.add_handler( request::resolve_element( "groupNames" ) );
    request.add_handler( [this, slug = move( slug )]( auto& data ) { on_groups_( move( slug ), move( data ) ); } );
    send( move( request ) );
}

void service::on_disconnect( disconnect_event::slot_type const& handler )
{
    on_disconnect_.connect( handler );
}

void service::on_temperature( temperature_event::slot_type const& handler )
{
    on_temperature_.connect( handler );
}

void service::on_printers( printers_event::slot_type const& handler )
{
    on_printers_.connect( handler );
}

void service::on_groups( groups_event::slot_type const& handler )
{
    on_groups_.connect( handler );
}

void service::connect()
{
    logger.info( "initiating connection to server" );

    client_ = make_unique< client >( context_, [this]( auto ec ) { this->handle_error( ec ); } );
    client_->subscribe( "temp", [this]( auto slug, auto data ) { on_temperature_( move( slug ), move( data ) ); } );
    client_->subscribe( "printerListChanged", [this]( auto, auto data ) { on_printers_( move( data ) ); } );
    client_->connect( settings_, [this] { this->handle_connected(); } );
}

void service::send( request&& req )
{
    queued_.push_back( move( req ) );
    if ( queued_.size() == 1 ) {
        send_next();
    }
}

void service::send_next()
{
    if ( connected_ && !queued_.empty() ) {
        auto& req = queued_.front();
        req.add_handler( [this] { queued_.pop_front(); this->send_next(); } );
        client_->send( move( req ) );
    }
}

void service::handle_connected()
{
    connected_ = true;
    retry_ = 0;
    send_next();
}

void service::handle_error( error_code ec )
{
    connected_ = false;
    client_ = nullptr;

    on_disconnect_( ec );

    long timeout = detail::retryTimeout( retry_++ );

    logger.error( "error in server communication, reconnecting in ", timeout, " seconds: ", ec.message() );

    auto timer = make_shared< asio::deadline_timer >( context_, boost::posix_time::seconds( timeout ) );
    timer->async_wait( [this, timer]( auto ) { this->connect(); } );
}

} // namespace rep
} // namespace prnet