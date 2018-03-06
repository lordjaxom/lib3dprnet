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

service::service( asio::io_context &context, settings settings, connect_callback cb )
        : context_( context )
        , settings_( move( settings ) )
        , connectCallback_( move( cb ) )
{
    connect();
}

service::~service() = default;

void service::list_printers( printers_callback cb )
{
    request req { "listPrinter" };
    req.add_handler( move( cb ) );
    send( move( req ) );
}

void service::list_groups( string printer, groups_callback cb )
{
    request request( "listModelGroups" );
    request.printer( move( printer ) );
    request.add_handler( request::check_ok_flag() );
    request.add_handler( request::resolve_element( "groupNames" ) );
    request.add_handler( move( cb ) );
    send( move( request ) );
}

void service::connect()
{
    logger.info( "initiating connection to server" );

    client_ = make_unique< client >( context_, [this]( auto ec ) { this->handle_error( ec ); } );
    client_->subscribe( "temp", [this]( auto printer, auto data ) { temperature( move( printer ), move( data ) ); } );
    client_->subscribe( "printerListChanged", [this]( auto printer, auto data ) { printers_changed( move( data ) ); } );
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

    if ( connectCallback_ ) {
        connectCallback_( {} );
    }

    send_next();
}

void service::handle_error( std::error_code ec )
{
    connected_ = false;
    client_ = nullptr;

    if ( connectCallback_ ) {
        connectCallback_( ec );
    }

    long timeout = detail::retryTimeout( retry_++ );

    logger.error( "error in server communication, reconnecting in ", timeout, " seconds: ", ec.message() );

    auto timer = make_shared< asio::deadline_timer >( context_, boost::posix_time::seconds( timeout ) );
    timer->async_wait( [this, timer]( auto ) { this->connect(); } );
}

} // namespace rep
} // namespace prnet