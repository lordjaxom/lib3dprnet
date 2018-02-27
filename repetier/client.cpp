#include <iostream> // FIXME
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

void client::connect( settings const& settings, callback<> callback )
{
    if ( socket_ ) {
        throw invalid_argument( "rep::client already connected" );
    }
    socket_ = make_unique< socket_impl >( context_ );

    asio::spawn( context_, [&, callback { move( callback ) }]( auto yield ) mutable {
        error_code ec;
        try {
            logger.info( "connecting to ", settings.host(), ":", settings.port() );

            tcp::resolver resolver { context_ };
            auto resolved { resolver.async_resolve( settings.host(), settings.port(), yield ) };

            asio::async_connect( ( *socket_ )->next_layer(), resolved, yield );
            ( *socket_ )->async_handshake( settings.host(), "/socket", yield );

            logger.debug( "connection successful, sending login request" );

            request request( "login", move( callback ) );
            request.set( "apikey", settings.apikey() );
            request.add_handler( request::check_ok_flag() );
            this->send( move( request ) );

            this->receive();
        } catch ( system_error const& e ) {
            ec = e.code();
        } catch ( boost::beast::system_error const& e ) {
            ec = e.code();
        }
        if ( ec ) {
            errorCallback_( ec );
        }
    } );
}

void client::send( request request )
{
    asio::spawn( context_, [&, request { move( request ) }] ( auto yield ) mutable {
        error_code ec;
        try {
            auto callbackId { ++lastCallbackId_ };
            request.callback_id( callbackId );

            auto message { request.dump() };

            logger.debug( ">>> ", message );

            ( *socket_  )->async_write( asio::buffer( message ), yield );
            pending_.emplace( callbackId, move( request ) );
        } catch ( system_error const& e ) {
            ec = e.code();
        } catch ( boost::beast::system_error const& e ) {
            ec = e.code();
        }
        if ( ec ) {
            errorCallback_( ec );
        }
    } );
}

void client::close()
{
    asio::spawn( context_, [&]( auto yield ) {
        logger.info( "closing connection to server" );

        try {
            ( *socket_ )->async_close( websocket::close_code::normal, yield );
            logger.debug( "destroying socket" );
            socket_ = nullptr;
        } catch ( exception const& e ) {
            logger.error( "caught while closing: ", e.what() );
        }
    } );
}

void client::receive()
{
    asio::spawn( context_, [&]( auto yield ) {
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
            ec = e.code();
        } catch ( boost::beast::system_error const& e ) {
            ec = e.code();
        } catch ( exception const& e ) {
            ec = make_error_code( prnet_errc::exception );
        }
        if ( ec ) {
            errorCallback_( ec );
        }
    } );
}

void client::handle_message( json const& message )
{
    if ( message.is_array() ) {
        for ( auto const& item : message ) {
            handle_message( item );
        }
        return;
    }

    json::const_iterator callbackId;
    if ( !message.is_object() ||
            ( callbackId = message.find( "callback_id" ) ) == message.cend() ||
            !callbackId->is_number() ) {
        logger.error( "incoming message is not a valid object containing callback_id" );
        throw system_error( make_error_code( prnet_errc::protocol_violation ) );
    }
    if ( *callbackId >= 0 ) {
        this->handle_callback( static_cast< size_t >( *callbackId ), message );
    }
}

void client::handle_callback( size_t callbackId, json const& message )
{
    auto pending { pending_.find( callbackId ) };
    if ( pending == pending_.end() ) {
        logger.error( "spurious callback ", callbackId, " received" );
        return;
    }

    auto data { message.find( "data" ) };
    if ( data == message.cend() ) {
        logger.error( "callback message ", callbackId, " does not contain valid data element" );
    }
    pending->second.handle( *data );
    pending_.erase( pending );
}

} // namespace rep
} // namespace prnet