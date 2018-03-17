#ifndef LIB3DPRNET_REPETIER_SOCKET_HPP
#define LIB3DPRNET_REPETIER_SOCKET_HPP

#include <cstddef>
#include <functional>
#include <memory>
#include <string>

#include <nlohmann/json_fwd.hpp>

#include "core/config.hpp"
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
    struct Pending;
    class Impl;

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
    std::unique_ptr< Impl > impl_;
};

} // namespace rep
} // namespace prnet

#endif // LIB3DPRNET_REPETIER_SOCKET_HPP
