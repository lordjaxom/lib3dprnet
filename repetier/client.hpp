#ifndef LIB3DPRNET_REPETIER_SOCKET_HPP
#define LIB3DPRNET_REPETIER_SOCKET_HPP

#include <cstddef>
#include <unordered_map>
#include <utility>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <nlohmann/json_fwd.hpp>

#include "core/config.hpp"
#include "core/string_view.hpp"
#include "forward.hpp"
#include "request.hpp"
#include "types.hpp"

namespace prnet {
namespace rep {

/**
 * class socket
 */

class PRNET_DLL client
{
public:
    using success_callback = std::function< void () >;
    using error_callback = std::function< void ( std::error_code ec ) >;
    using event_callback = std::function< void ( std::string printer, nlohmann::json data ) >;

    client( boost::asio::io_context& context, error_callback ecb );
    client( client const& ) = delete;
    ~client();

    void connect( settings set, success_callback cb );
    void send( request req );
    void close();

    void subscribe( std::string event, event_callback cb );

private:
    template< typename Func >
    void checked_spawn( Func&& func );

    void receive();

    void handle_message( nlohmann::json&& message );
    void handle_callback( std::size_t callbackId, nlohmann::json&& data );
    void handle_event( nlohmann::json&& event );

    boost::asio::io_context& context_;
    boost::beast::websocket::stream< boost::asio::ip::tcp::socket > stream_;
    error_callback errorCallback_;
    std::unordered_map< std::size_t, request > pending_;
    std::unordered_map< std::string, event_callback > subscriptions_;
    std::size_t lastCallbackId_ {};
};

} // namespace rep
} // namespace prnet

#endif // LIB3DPRNET_REPETIER_SOCKET_HPP
