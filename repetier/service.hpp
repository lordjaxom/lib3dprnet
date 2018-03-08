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

/**
 * class service
 */

class PRNET_DLL service
{
    using disconnect_event = boost::signals2::signal< void ( std::error_code ec ) >;
    using temperature_event = boost::signals2::signal< void ( std::string slug, temperature temp ) >;
    using printers_event = boost::signals2::signal< void ( std::vector< printer > printers ) >;
    using groups_event = boost::signals2::signal< void ( std::string printer, std::vector< group > groups ) >;

public:
    service( boost::asio::io_context& context, settings settings );
    service( service const& ) = delete;
    ~service();

    void request_printers();
    void request_groups( std::string slug );

    void on_disconnect( disconnect_event::slot_type const& handler );
    void on_temperature( temperature_event::slot_type const& handler );
    void on_printers( printers_event::slot_type const& handler );
    void on_groups( groups_event::slot_type const& handler );

private:
    void connect();
    void send( request&& req );
    void send_next();

    void handle_connected();
    void handle_error( std::error_code ec );

    boost::asio::io_context& context_;
    settings settings_;
    std::unique_ptr< client > client_;
    bool connected_ {};
    std::size_t retry_ {};
    std::list< request > queued_;

    disconnect_event on_disconnect_;
    temperature_event on_temperature_;
    printers_event on_printers_;
    groups_event on_groups_;
};

} // namespace rep
} // namespace prnet

#endif // LIB3DPRNET_REPETIER_CLIENT_HPP
