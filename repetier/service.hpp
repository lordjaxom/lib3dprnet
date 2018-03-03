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
    using job_event = event< std::string >;
    using temperature_event = event< std::string, temperature_info >;

    service( boost::asio::io_context& context, settings settings );
    ~service();

    void list_printer( callback< std::vector< printer > > cb );
    void list_groups( std::string printer, callback< std::vector< group > > cb );

    job_event job_started;
    job_event job_finished;
    job_event job_killed;
    temperature_event temperature;

private:
    void connect();
    void send( request&& req );
    void send_next();

    void handle_connected();
    bool handle_error( std::error_code ec );

    boost::asio::io_context& context_;
    settings settings_;
    std::unique_ptr< client > client_;
    bool connected_ {};
    bool pending_ {};
    std::list< request > queued_;
};

} // namespace rep
} // namespace prnet

#endif // LIB3DPRNET_REPETIER_CLIENT_HPP
