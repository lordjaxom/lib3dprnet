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

void service::connect()
{
    logger.info( "initiating connection to server" );

    client_ = make_unique< client >( context_, [this]( auto ec ) { this->handle_error( ec ); } );
    client_->connect( settings_, [this] { this->handle_connected(); } );
}

void service::list_printer( callback< vector< printer > > callback )
{
    request request { "listPrinter", move( callback ) };
    send_or_queue( move( request ) );
}

void service::list_groups( std::string printer, callback< std::vector< group > > callback )
{
    request request { "listModelGroups", [callback { move( callback ) }]( auto const& data ) {
        auto groupNames { data.find( "groupNames" ) };
        if ( groupNames == data.end() || !groupNames->is_array() ) {
            logger.error( "data object does not contain valid groupNames array" );
            throw system_error( make_error_code( prnet_errc::protocol_violation ) );
        }
    } };
    request.printer( move( printer ) );
    request.add_handler( request::check_ok_flag() );
    send_or_queue( move( request ) );
}

void service::handle_connected()
{
    connected_ = true;
    while ( !queued_.empty() ) {
        client_->send( move( queued_.front() ) );
        queued_.pop_front();
    }
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

void service::send_or_queue( request&& request )
{
    if ( connected_ ) {
        client_->send( move( request ) );
    } else {
        queued_.push_back( move( request ) );
    }
}

} // namespace rep
} // namespace prnet