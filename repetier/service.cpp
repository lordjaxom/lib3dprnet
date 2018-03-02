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

service::service( boost::asio::io_context &context, settings settings )
        : context_ { context }
        , settings_ { move( settings ) }
{
    connect();
}

service::~service() = default;

void service::list_printer( callback< vector< printer > > cb )
{
    request request { "listPrinter", move( cb ) };
    send( move( request ) );
}

void service::list_groups( string printer, callback< vector< group > > cb )
{
    request request { "listModelGroups", [cb { move( cb ) }]( auto data ) { cb( move( data[ "groupNames" ] ) ); } };
    request.printer( move( printer ) );
    request.add_handler( request::check_ok_flag() );
    send( move( request ) );
}

void service::connect()
{
    logger.info( "initiating connection to server" );

    client_ = make_unique< client >( context_, [this]( auto ec ) { this->handle_error( ec ); } );
    client_->subscribe( "jobStarted", [this]( auto printer, auto ) { job_started( move( printer ) ); } );
    client_->subscribe( "jobFinished", [this]( auto printer, auto ) { job_finished( move( printer ) ); } );
    client_->subscribe( "jobKilled", [this]( auto printer, auto ) { job_killed( move( printer ) ); } );
    client_->connect( settings_, [this] { this->handle_connected(); } );
}

void service::send( request&& req )
{
    req.add_handler( [this]( auto& ) { pending_ = false; this->send_next(); } );
    queued_.push_back( move( req ) );
    send_next();
}

void service::send_next()
{
    if ( !queued_.empty() && connected_ && !pending_ ) {
        client_->send( move( queued_.front() ) );
        queued_.pop_front();
        pending_ = true;
    }
}

void service::handle_connected()
{
    connected_ = true;
    send_next();
}

bool service::handle_error( std::error_code ec )
{
    if ( !ec ) {
        return true;
    }

    logger.error( "error in server communication, reconnecting in five seconds: ", ec.message() );

    connected_ = false;
    client_ = {};

    auto timer = make_shared< asio::deadline_timer >( context_, boost::posix_time::seconds( 5 ) );
    timer->async_wait( [this, timer]( auto ec ) { this->connect(); } );
    return false;
}

} // namespace rep
} // namespace prnet