#ifndef LIB3DPRNET_REPETIER_CLIENT_HPP
#define LIB3DPRNET_REPETIER_CLIENT_HPP

#include <list>
#include <memory>
#include <string>
#include <system_error>
#include <vector>

#include <boost/asio/io_context.hpp>

#include "core/config.hpp"
#include "forward.hpp"
#include "types.hpp"

namespace prnet {
namespace rep {

class PRNET_DLL service
{
public:
    service( boost::asio::io_context& context, settings settings );
    ~service();

    void list_printer( callback< std::vector< printer > > cb );
    void list_groups( std::string printer, callback< std::vector< group > > cb );

private:
    void connect();
    void send_or_queue( request&& request );

    void handle_connected();
    bool handle_error( std::error_code ec );

    boost::asio::io_context& context_;
    settings settings_;
    std::unique_ptr< client > client_;
    bool connected_ {};
    std::list< request > queued_;
};

} // namespace rep
} // namespace prnet

#endif // LIB3DPRNET_REPETIER_CLIENT_HPP
