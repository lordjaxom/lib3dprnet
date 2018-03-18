#include <cassert>
#include <algorithm>
#include <stdexcept>
#include <unordered_map>

#include <boost/asio/buffers_iterator.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/multi_buffer.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <nlohmann/json.hpp>

#include "3dprnet/core/error.hpp"
#include "3dprnet/core/logging.hpp"
#include "3dprnet/core/optional.hpp"
#include "3dprnet/repetier/client.hpp"
#include "3dprnet/repetier/types.hpp"

using namespace std;
using namespace nlohmann;

namespace asio = boost::asio;
namespace websocket = boost::beast::websocket;

using tcp = asio::ip::tcp;

namespace prnet {
namespace rep {

static Logger logger( "rep::Client" );

/**
 * class Client
 */

struct Client::Pending
{
    Pending( size_t callbackId, CallbackHandler&& handler, asio::steady_timer&& timer )
            : callbackId( callbackId )
            , handler( move( handler ) )
            , timer( move( timer ) ) {}

    size_t callbackId;
    Client::CallbackHandler handler;
    asio::steady_timer timer;
};

class Client::Impl
{
public:
    Impl( asio::io_context& context, ErrorHandler&& errorHandler )
            : context_( context )
            , errorHandler_( move( errorHandler ) )
            , stream_( context_ ) {}

    void connect( Endpoint&& endpoint, SuccessHandler&& handler )
    {
        assert( !connected_ );

        logger.debug( "before spawn" );

        checked_spawn( [this, endpoint = move( endpoint ), handler = move( handler )]( auto yield ) {
            logger.info( "connecting to ", endpoint.host(), ":", endpoint.port() );

            tcp::resolver resolver( context_ );
            auto resolved = resolver.async_resolve( endpoint.host(), endpoint.port(), yield );

            asio::async_connect( stream_.next_layer(), resolved, yield );
            stream_.async_handshake( endpoint.host(), "/socket", yield );

            logger.debug( "connection successfully established" );

            connected_ = true;
            handler();

            this->receive();
        } );
    }

    void send( json& request, CallbackHandler&& handler )
    {
        assert( connected_ );
        assert( pending_ == nullopt );

        checked_spawn( [this, &request, handler = move( handler )]( auto yield ) mutable {
            auto callbackId = ++lastCallbackId_;
            request[ "callback_id" ] = callbackId;
            auto message = request.dump();

            logger.debug( ">>> ", message );

            stream_.async_write( asio::buffer( message ), yield );
            pending_ = Pending( callbackId, move( handler ), { context_, chrono::seconds( 5 ) } ); // TODO
            pending_->timer.async_wait( [this, callbackId]( error_code ec ) {
                this->handle_timeout( callbackId, ec );
            } );
        } );
    }

    void close()
    {
        assert( connected_ );

        checked_spawn( [this]( auto yield ) {
            logger.info( "closing connection to server" );

            stream_.async_close( websocket::close_code::normal, yield );
            stream_.next_layer().close();
            pending_ = nullopt;
            connected_ = false;
        } );
    }

    void subscribe( string&& event, EventHandler&& handler )
    {
        subscriptions_.emplace( move( event ), move( handler ) );
    }

private:
    template< typename Func >
    void checked_spawn( Func&& func )
    {
        asio::spawn( context_, [this, func = move( func )]( auto yield ) mutable {
            try {
                func( yield );
            } catch ( system_error const& e ) {
                this->handle_error( e.code() );
            } catch ( boost::beast::system_error const& e ) {
                this->handle_error( e.code() );
            } catch ( json::exception const& e ) {
                logger.warning( "protocol violation from server: ", e.what() );
            }
        } );
    }

    void receive()
    {
        checked_spawn( [this]( auto yield ) {
            boost::beast::multi_buffer buffer;
            stream_.async_read( buffer, yield );

            // for some reason the message must be one contiguous sequence for json::parse
            auto message = boost::beast::buffers_to_string( buffer.data() );

            logger.debug( "<<< ", message );

            this->handle_message( json::parse( message ) );
            this->receive();
        } );
    }

    void handle_message( json const& message )
    {
        long callbackId = message.at( "callback_id" );
        auto const& data = message.at( "data" );
        if ( callbackId >= 0 ) {
            handle_callback( static_cast< size_t >( callbackId ), data );
        } else if ( message.value( "eventList", false ) ) {
            for_each( data.begin(), data.end(), [this]( auto const& event ) { this->handle_event( event ); } );
        }
    }

    void handle_callback( size_t callbackId, json const& data )
    {
        if ( pending_ == nullopt ) {
            logger.error( "received callback ", callbackId, " although no pending request exists" );
            return;
        }
        if ( pending_->callbackId != callbackId ) {
            logger.error( "received callback ", callbackId, " although waiting for callback ", pending_->callbackId );
            return;
        }

        auto handler = move( pending_->handler );
        pending_ = nullopt;
        handler( data );
    }

    void handle_event( json const& event )
    {
        string eventType = event.at( "event" );
        auto subscription = subscriptions_.find( eventType );
        if ( subscription != subscriptions_.end() ) {
            subscription->second( event.value( "printer", "" ), event.at( "data" ) );
        }
    }

    void handle_error( error_code ec )
    {
        if ( ec != make_error_code( asio::error::operation_aborted ) ) {
            logger.error( "error communicating with server: ", ec.message() );

            connected_ = false;
            errorHandler_( ec );
        }
    }

    void handle_timeout( size_t callbackId, error_code ec )
    {
        if ( ec == make_error_code( asio::error::operation_aborted ) ) {
            return;
        }

        if ( ec ) {
            logger.error( "error waiting for callback ", callbackId, ": ", ec.message() );
        } else {
            logger.error( "timeout waiting for callback ", callbackId );
            ec = make_error_code( prnet_errc::timeout );
        }

        pending_ = nullopt;
        errorHandler_( ec );
    }

    asio::io_context& context_;
    ErrorHandler errorHandler_;
    boost::beast::websocket::stream< asio::ip::tcp::socket > stream_;
    bool connected_ {};
    optional< Pending > pending_;
    unordered_map< string, EventHandler > subscriptions_;
    size_t lastCallbackId_ {};
};

Client::Client( asio::io_context& context, ErrorHandler handler )
        : impl_( make_unique< Impl >( context, move( handler ) ) ) {}

Client::~Client() = default;

void Client::connect( Endpoint endpoint, SuccessHandler handler )
{
    impl_->connect( move( endpoint ), move( handler ) );
}

void Client::send( json& request, CallbackHandler handler )
{
    impl_->send( request, move( handler ) );
}

void Client::close()
{
    impl_->close();
}

void Client::subscribe( string event, EventHandler handler )
{
    impl_->subscribe( move( event ), move( handler ) );
}

} // namespace rep
} // namespace prnet
