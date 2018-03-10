#ifndef LIB3DPRNET_REPETIER_CLIENT_HPP
#define LIB3DPRNET_REPETIER_CLIENT_HPP

#include <list>
#include <memory>
#include <string>
#include <system_error>
#include <vector>

#include <boost/asio/io_context.hpp>
#include <boost/signals2/signal.hpp>
#include <nlohmann/json.hpp>

#include "core/config.hpp"
#include "client.hpp"
#include "forward.hpp"
#include "types.hpp"

namespace prnet {
namespace rep {

/**
 * class Service
 */

class PRNET_DLL Service
{
public:
	using CallbackHandler = Client::CallbackHandler;

	using reconnect_event = boost::signals2::signal< void () >;
    using disconnect_event = boost::signals2::signal< void ( std::error_code ec ) >;
    using temperature_event = boost::signals2::signal< void ( std::string slug, temperature temp ) >;
    using printers_event = boost::signals2::signal< void ( std::vector< printer > printers ) >;
    using config_event = boost::signals2::signal< void ( std::string slug, printer_config config ) >;
    using groups_event = boost::signals2::signal< void ( std::string slug, std::vector< group > groups ) >;
    using models_event = boost::signals2::signal< void ( std::string slug, std::vector< model > models ) >;

private:
	struct Action
	{
		Action( nlohmann::json&& request, CallbackHandler&& handler );

		nlohmann::json request;
		CallbackHandler handler;
	};

public:
    Service( boost::asio::io_context& context, Endpoint endpoint );
    Service( Service const& ) = delete;
    ~Service();

    bool connected() const { return connected_; }

    void request_printers();
    void request_config( std::string slug );
    void request_groups( std::string slug );
    void request_models( std::string slug );

    void on_reconnect( reconnect_event::slot_type const& handler );
    void on_disconnect( disconnect_event::slot_type const& handler );
    void on_temperature( temperature_event::slot_type const& handler );
    void on_printers( printers_event::slot_type const& handler );
    void on_config( config_event::slot_type const& handler );
    void on_groups( groups_event::slot_type const& handler );
    void on_models( models_event::slot_type const& handler );

private:
    void connect();
    void send( nlohmann::json&& request, CallbackHandler handler, bool priority = false );
    void send_next( bool force = false );

    void handle_connected();
	void handle_login();
	void handle_sent();
    void handle_error( std::error_code ec );

    boost::asio::io_context& context_;
    Endpoint endpoint_;
    std::unique_ptr< Client > client_;
    bool connected_ {};
	bool pending_ {};
    std::size_t retry_ {};
    std::list< Action > queued_;

    reconnect_event on_reconnect_;
    disconnect_event on_disconnect_;
    temperature_event on_temperature_;
    printers_event on_printers_;
    config_event on_config_;
    groups_event on_groups_;
    models_event on_models_;
};

} // namespace rep
} // namespace prnet

#endif // LIB3DPRNET_REPETIER_CLIENT_HPP
