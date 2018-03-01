#ifndef LIB3DPRNET_REPETIER_SOCKET_HPP
#define LIB3DPRNET_REPETIER_SOCKET_HPP

#include <cstddef>
#include <memory>
#include <unordered_map>

#include <boost/asio/io_context.hpp>
#include <boost/asio/io_context_strand.hpp>
#include <nlohmann/json_fwd.hpp>

#include "core/config.hpp"
#include "forward.hpp"
#include "request.hpp"

namespace prnet {
namespace rep {

/**
 * class socket
 */

class PRNET_DLL client
{
    struct socket_impl;

public:
    explicit client( boost::asio::io_context& context, error_callback errorCallback );
    ~client();

    void connect( settings const& settings, callback<> cb );
    void send( request req );
    void close();

private:
    void receive();
    void handle_message( nlohmann::json&& message );
    void handle_callback( std::size_t callbackId, nlohmann::json&& message );

    boost::asio::io_context& context_;
    error_callback errorCallback_;
    std::unique_ptr< socket_impl > socket_;
    std::unordered_map< std::size_t, request > pending_;
    std::size_t lastCallbackId_ {};
    bool closing_;
};

} // namespace rep
} // namespace prnet

#endif // LIB3DPRNET_REPETIER_SOCKET_HPP
