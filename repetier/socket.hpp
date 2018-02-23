#ifndef LIB3DPRNET_REPETIER_SOCKET_HPP
#define LIB3DPRNET_REPETIER_SOCKET_HPP

#include <cstddef>
#include <memory>
#include <unordered_map>

#include <boost/asio/io_context.hpp>

#include "request.hpp"
#include "types.hpp"

namespace prnet {
namespace rep {

/**
 * class socket
 */

class socket
{
    struct socket_impl;

public:
    socket( boost::asio::io_context& context );
    ~socket();

    void connect( settings const& settings, callback<> callback );

private:
    void receive();
    void send( request request );

    boost::asio::io_context& context_;
    std::unique_ptr< socket_impl > socket_;
    std::unordered_map< std::size_t, request > pending_;
    std::size_t lastCallbackId_ {};
};

} // namespace rep
} // namespace prnet

#endif // LIB3DPRNET_REPETIER_SOCKET_HPP
