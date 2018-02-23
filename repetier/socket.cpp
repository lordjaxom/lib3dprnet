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

#include "request.hpp"
#include "socket.hpp"

using namespace std;
using namespace nlohmann;

namespace asio = boost::asio;
namespace websocket = boost::beast::websocket;

using tcp = asio::ip::tcp;

namespace prnet {
namespace rep {

/**
 * struct socket::socket_impl
 */

struct socket::socket_impl
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

socket::socket( asio::io_context& context )
        : context_ { context } {}

socket::~socket() = default;

void socket::connect( settings const& settings, callback<> callback )
{
    if ( socket_ ) {
        throw invalid_argument( "rep::socket already connected" );
    }
    socket_ = make_unique< socket_impl >( context_ );

    asio::spawn( context_, [&, callback { move( callback ) }]( auto yield ) {
        error_code ec;
        try {
            tcp::resolver resolver { context_ };
            auto resolved { resolver.async_resolve( settings.host(), settings.port(), yield ) };

            asio::async_connect( ( *socket_ )->next_layer(), resolved, yield );
            ( *socket_ )->async_handshake( settings.host(), "/socket", yield );

            request request( "login", move( callback ) );
            request.set( "apikey", settings.apikey() );
            request.add_handler( request::check_ok_flag() );
            this->send( move( request ) );

            this->receive();

            return;
        } catch ( system_error const& e ) {
            ec = e.code();
        } catch ( boost::beast::system_error const& e ) {
            ec = e.code();
        }
        callback( ec );
    } );
}

void socket::receive()
{
    asio::spawn( context_, [&]( auto yield ) {
        error_code ec;
        try {
            boost::beast::multi_buffer buffer;
            ( *socket_ )->async_read( buffer, yield );

            // for some reason the message must be one contiguous sequence for json::parse
            string message { asio::buffers_begin( buffer.data() ), asio::buffers_end( buffer.data() ) };
            auto json { json::parse( message ) };

            std::cout << "<<< " << message << std::endl;

            this->receive();
        } catch ( system_error const& e ) {
            ec = e.code();
        } catch ( boost::beast::system_error const& e ) {
            ec = e.code();
        }
    } );
}

void socket::send( request request )
{
    asio::spawn( context_, [&, request { move( request ) }] ( auto yield ) mutable {
        error_code ec;
        try {
            auto callbackId { ++lastCallbackId_ };
            request.set( "callback_id", callbackId );

            auto message { request.dump() };

            std::cout << ">>> " << message << std::endl;

            ( *socket_  )->async_write( asio::buffer( message ), yield );
            pending_.emplace( callbackId, move( request ) );

            return;
        } catch ( system_error const& e ) {
            ec = e.code();
        } catch ( boost::beast::system_error const& e ) {
            ec = e.code();
        }
        request.error( ec );
    } );
}

} // namespace rep
} // namespace prnet