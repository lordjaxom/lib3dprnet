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
#include <core/error.hpp>

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
 * struct socket::socket_impl
 */

struct client::socket_impl
{
    using impl_type = websocket::stream< tcp::socket >;

    socket_impl( asio::io_context& context )
            : socket_ { context } {}

    impl_type& operator*() { return socket_; }
    impl_type* operator->() { return &socket_; }

private:
    websocket::stream< tcp::socket > socket_;
};


/**
 * class socket
 */

client::client( asio::io_context& context, error_callback errorCallback )
        : context_ { context }
        , errorCallback_ { move( errorCallback ) } {}

client::~client() = default;

void client::connect( settings const& settings, callback<> cb )
{
    if ( socket_ ) {
        throw invalid_argument( "rep::client already connected" );
    }
    socket_ = make_unique< socket_impl >( context_ );
    closing_ = false;

    asio::spawn( context_, [&, cb { move( cb ) }]( auto yield ) mutable {
        try {
            logger.info( "connecting to ", settings.host(), ":", settings.port() );

            tcp::resolver resolver { context_ };
            auto resolved { resolver.async_resolve( settings.host(), settings.port(), yield ) };

            asio::async_connect( ( *socket_ )->next_layer(), resolved, yield );
            ( *socket_ )->async_handshake( settings.host(), "/socket", yield );

            logger.debug( "connection successful, sending login request" );

            request req { "login", [cb { move( cb ) }]( auto ) { cb(); } };
            req.set( "apikey", settings.apikey() );
            req.add_handler( request::check_ok_flag() );
            this->send( move( req ) );

            this->receive();
        } catch ( system_error const& e ) {
            errorCallback_( e.code() );
        } catch ( boost::beast::system_error const& e ) {
            errorCallback_( e.code() );
        }
    } );
}

void client::send( request req )
{
    asio::spawn( context_, [&, req { move( req ) }] ( auto yield ) mutable {
        try {
            auto callbackId { ++lastCallbackId_ };
            req.callback_id( callbackId );

            auto message { req.dump() };

            logger.debug( ">>> ", message );

            ( *socket_  )->async_write( asio::buffer( message ), yield );
            pending_.emplace( callbackId, move( req ) );
        } catch ( system_error const& e ) {
            errorCallback_( e.code() );
        } catch ( boost::beast::system_error const& e ) {
            errorCallback_( e.code() );
        }
    } );
}

void client::close()
{
    asio::spawn( context_, [&]( auto yield ) {
        logger.info( "closing connection to server" );

        try {
            closing_ = true;
            ( *socket_ )->async_close( websocket::close_code::normal, yield );
        } catch ( system_error const& e ) {
            // ignore
        } catch ( boost::beast::system_error const& e ) {
            // ignore
        }
        socket_ = nullptr;
    } );
}

void client::subscribe( string event, event_callback cb )
{
    subscriptions_.emplace( move( event ), move( cb ) );
}

void client::receive()
{
    if ( closing_ ) {
        return;
    }

    asio::spawn( context_, [this]( auto yield ) {
        error_code ec;
        try {
            boost::beast::multi_buffer buffer;
            ( *socket_ )->async_read( buffer, yield );

            // for some reason the message must be one contiguous sequence for json::parse
            string message { asio::buffers_begin( buffer.data() ), asio::buffers_end( buffer.data() ) };

            logger.debug( "<<< ", message );

            this->handle_message( json::parse( message ) );
            this->receive();
        } catch ( system_error const& e ) {
            errorCallback_( e.code() );
        } catch ( boost::beast::system_error const& e ) {
            if ( e.code() == make_error_code( boost::asio::error::operation_aborted ) && closing_ ) {
                return;
            }
            errorCallback_( e.code() );
        } catch ( json::exception const& e ) {
            logger.error( "protocol violation in message from server: ", e.what() );
            errorCallback_( make_error_code( prnet_errc::protocol_violation ) );
        }
    } );
}

void client::handle_message( json&& message )
{
    if ( message.is_array() ) {
        for_each( message.begin(), message.end(), [this]( auto& item ) { this->handle_message( move( item ) ); } );
        return;
    }

    long callbackId { message[ "callback_id" ] };
    auto& data { message[ "data" ] };
    if ( callbackId >= 0 ) {
        handle_callback( static_cast< size_t >( callbackId ), move( data ) );
    } else if ( message[ "eventList" ] ) {
        for_each( data.begin(), data.end(), [this]( auto& event ) { this->handle_event( move( event ) ); } );
    }
}

void client::handle_callback( size_t callbackId, json&& data )
{
    auto pending { pending_.find( callbackId ) };
    if ( pending == pending_.end() ) {
        logger.error( "spurious callback ", callbackId, " received" );
        return;
    }
    pending->second.handle( move( data ) );
    pending_.erase( pending );
}

void client::handle_event( json&& event )
{
    auto& type { event[ "event" ] };
    auto subscription { subscriptions_.find( type ) };
    if ( subscription != subscriptions_.end() ) {
        subscription->second( move( event[ "printer" ] ), move( event[ "data" ] ) );
    }
}

} // namespace rep
} // namespace prnet