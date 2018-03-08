#include <cassert>
#include <algorithm>
#include <stdexcept>
#include <unordered_map>

#include <boost/asio/buffers_iterator.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/multi_buffer.hpp>
#include <boost/beast/websocket/stream.hpp>

#include "core/error.hpp"
#include "core/logging.hpp"
#include "request.hpp"
#include "client.hpp"
#include "types.hpp"

using namespace std;
using namespace nlohmann;

namespace asio = boost::asio;
namespace websocket = boost::beast::websocket;

using tcp = asio::ip::tcp;

namespace prnet {
namespace rep {

static logger logger( "rep::client" );


/**
 * class socket
 */

client::client( asio::io_context& context, error_callback ecb )
        : context_ { context }
        , stream_ { context_ }
        , errorCallback_ { move( ecb ) } {}

client::~client() = default;

void client::connect( settings set, success_callback cb )
{
    checked_spawn( [this, set = move( set ), cb = move( cb )]( auto yield ) mutable {
        logger.info( "connecting to ", set.host(), ":", set.port() );

        tcp::resolver resolver { context_ };
        auto resolved { resolver.async_resolve( set.host(), set.port(), yield ) };

        asio::async_connect( stream_.next_layer(), resolved, yield );
        stream_.async_handshake( set.host(), "/socket", yield );

        logger.debug( "connection successful, sending login request" );

        request req( *this, "login" );
        req.set( "apikey", set.apikey() );
        req.add_handler( request::check_ok_flag() );
        req.add_handler( move( cb ) );

        this->send( move( req ) );

        this->receive();
    } );
}

void client::send( request req )
{
    checked_spawn( [this, req { move( req ) }] ( auto yield ) mutable {
        auto message { req.dump() };

        logger.debug( ">>> ", message );

        stream_.async_write( asio::buffer( message ), yield );
        pending_.emplace( req.callbackId(), move( req ) );
    } );
}

void client::close()
{
    checked_spawn( [&]( auto yield ) {
        logger.info( "closing connection to server" );

        stream_.async_close( websocket::close_code::normal, yield );
    } );
}

void client::subscribe( string event, event_callback cb )
{
    subscriptions_.emplace( move( event ), move( cb ) );
}

template< typename Func >
void client::checked_spawn( Func&& func )
{
    asio::spawn( context_, [this, func { move( func ) }]( auto yield ) mutable {
        try {
            func( yield );
        } catch ( system_error const& e ) {
            logger.error( "error communicating with repetier server: ", e.what() );
            errorCallback_( e.code() );
        } catch ( boost::beast::system_error const& e ) {
            if ( e.code() != make_error_code( boost::asio::error::operation_aborted ) ) {
                logger.error( "error communicating with repetier server: ", e.what() );
                errorCallback_( e.code() );
            }
        } catch ( json::exception const& e ) {
            logger.warning( "protocol violation from repetier server: ", e.what() );
        }
    } );
}

void client::receive()
{
    checked_spawn( [this]( auto yield ) {
        boost::beast::multi_buffer buffer;
        stream_.async_read( buffer, yield );

        // for some reason the message must be one contiguous sequence for json::parse
        string message( asio::buffers_begin( buffer.data() ), asio::buffers_end( buffer.data() ) );

        logger.debug( "<<< ", message );

        this->handle_message( json::parse( message ) );
        this->receive();
    } );
}

void client::handle_message( json&& message )
{
    long callbackId = message.at( "callback_id" );
    auto& data = message.at( "data" );
    if ( callbackId >= 0 ) {
        handle_callback( static_cast< size_t >( callbackId ), move( data ) );
    } else if ( message.value( "eventList", false ) ) {
        for_each( data.begin(), data.end(), [this]( auto& event ) { this->handle_event( move( event ) ); } );
    }
}

void client::handle_callback( size_t callbackId, json&& data )
{
    auto pending = pending_.find( callbackId );
    if ( pending == pending_.end() ) {
        logger.error( "spurious callback ", callbackId, " received" );
        return;
    }
    pending->second.handle( move( data ) );
    pending_.erase( pending );
}

void client::handle_event( json&& event )
{
    string eventType = event.at( "event" );
    auto subscription = subscriptions_.find( eventType );
    if ( subscription != subscriptions_.end() ) {
        subscription->second( event.value( "printer", string() ), move( event.at( "data" ) ) );
    }
}

} // namespace rep
} // namespace prnet