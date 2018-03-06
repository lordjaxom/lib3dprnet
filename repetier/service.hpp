#ifndef LIB3DPRNET_REPETIER_CLIENT_HPP
#define LIB3DPRNET_REPETIER_CLIENT_HPP

#include <list>
#include <memory>
#include <string>
#include <system_error>
#include <vector>

#include <boost/asio/io_context.hpp>
#include <boost/signals2/signal.hpp>

#include "core/config.hpp"
#include "forward.hpp"
#include "types.hpp"

namespace prnet {
namespace rep {

template< typename ...Args >
using event = boost::signals2::signal< void ( Args... ) >;

/**
 * class service
 */

class PRNET_DLL service
{
public:
    using connect_callback = std::function< void ( std::error_code ec ) >;
    using printers_callback = std::function< void ( std::vector< printer > printers ) >;
    using groups_callback = std::function< void ( std::vector< group > groups ) >;

    using temperature_event = boost::signals2::signal< void ( std::string printer, temperature temp ) >;
    using printers_event = boost::signals2::signal< void ( std::vector< printer > printers ) >;

    service( boost::asio::io_context& context, settings settings, connect_callback cb = {} );
    service( service const& ) = delete;
    ~service();

    void list_printers( printers_callback cb );
    void list_groups( std::string printer, groups_callback cb );

    temperature_event temperature;
    printers_event printers_changed;

private:
    void connect();
    void send( request&& req );
    void send_next();

    void handle_connected();
    void handle_error( std::error_code ec );

    boost::asio::io_context& context_;
    settings settings_;
    connect_callback connectCallback_;
    std::unique_ptr< client > client_;
    bool connected_ {};
    std::size_t retry_ {};
    std::list< request > queued_;
};

} // namespace rep
} // namespace prnet

#endif // LIB3DPRNET_REPETIER_CLIENT_HPP
