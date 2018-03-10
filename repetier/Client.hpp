#ifndef LIB3DPRNET_REPETIER_SOCKET_HPP
#define LIB3DPRNET_REPETIER_SOCKET_HPP

#include <cstddef>
#include <unordered_map>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <boost/operators.hpp>
#include <nlohmann/json_fwd.hpp>

#include "core/config.hpp"
#include "core/optional.hpp"
#include "forward.hpp"
#include "types.hpp"

namespace prnet {
namespace rep {

/**
 * class Client
 */

class PRNET_DLL Client
{
public:
    using SuccessHandler = std::function< void () >;
    using ErrorHandler = std::function< void ( std::error_code ec ) >;
    using CallbackHandler = std::function< void ( nlohmann::json const& data ) >;
    using EventHandler = std::function< void ( std::string printer, nlohmann::json const& data ) >;

private:
    struct Pending
    {
        bool empty() const { return callbackId == 0; }

        std::size_t callbackId;
        CallbackHandler handler;
        optional< boost::asio::steady_timer > timer;
    };

public:
    /**
     * Constructs a client object without connecting. The handler will be called whenever there is an error during
     * communication. Using the client after the handler has been called results in undefined behaviour.
     */
    Client( boost::asio::io_context& context, ErrorHandler handler );
    Client( Client const& ) = delete;
    ~Client();

    /**
     * Attempts to connect to the server specified by endpoint. Will invoke handler upon successful connection. Using
     * the client before the handler has been called results in undefined behaviour.
     */
    void connect( Endpoint endpoint, SuccessHandler handler );

    /**
     * Sends request to the server and waits for a matching response. The request object only needs to be valid until
     * send returns. Sending another request before the handler has been called results in undefined behaviour.
     */
    void send( nlohmann::json& request, CallbackHandler handler );

    /**
     * TODO
     */
    void close();

    void subscribe( std::string event, EventHandler handler );

private:
    template< typename Func > void checked_spawn( Func&& func );

    void receive();

    void handle_message( nlohmann::json const& message );
    void handle_callback( std::size_t callbackId, nlohmann::json const& data );
    void handle_event( nlohmann::json const& event );
    void handle_error( std::error_code ec );
    void handle_timeout( std::size_t callbackId, std::error_code ec );

    boost::asio::io_context& context_;
    ErrorHandler errorHandler_;

    boost::beast::websocket::stream< boost::asio::ip::tcp::socket > stream_;
    bool connected_ {};
    Pending pending_ {};
    std::unordered_map< std::string, EventHandler > subscriptions_;
	std::size_t lastCallbackId_ {};
};

} // namespace rep
} // namespace prnet

#endif // LIB3DPRNET_REPETIER_SOCKET_HPP
