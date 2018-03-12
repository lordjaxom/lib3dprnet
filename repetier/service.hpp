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

	using ReconnectEvent = boost::signals2::signal< void () >;
    using DisconnectEvent = boost::signals2::signal< void ( std::error_code ec ) >;
    using TemperatureEvent = boost::signals2::signal< void ( std::string slug, temperature temp ) >;
    using PrintersEvent = boost::signals2::signal< void ( std::vector< printer > printers ) >;
    using ConfigEvent = boost::signals2::signal< void ( std::string slug, printer_config config ) >;
    using GroupsEvent = boost::signals2::signal< void ( std::string slug, std::vector< group > groups ) >;
    using ModelsEvent = boost::signals2::signal< void ( std::string slug, std::vector< model > models ) >;

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

	void add_model_group( std::string slug, std::string group );
    void delete_model_group( std::string slug, std::string group, bool deleteModels );
    void remove_model( std::string slug, std::size_t id );
    void move_model_to_group( std::string slug, std::size_t id, std::string group );

    void on_reconnect( ReconnectEvent::slot_type const& handler );
    void on_disconnect( DisconnectEvent::slot_type const& handler );
    void on_temperature( TemperatureEvent::slot_type const& handler );
    void on_printers( PrintersEvent::slot_type const& handler );
    void on_config( ConfigEvent::slot_type const& handler );
    void on_groups( GroupsEvent::slot_type const& handler );
    void on_models( ModelsEvent::slot_type const& handler );

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

    ReconnectEvent on_reconnect_;
    DisconnectEvent on_disconnect_;
    TemperatureEvent on_temperature_;
    PrintersEvent on_printers_;
    ConfigEvent on_config_;
    GroupsEvent on_groups_;
    ModelsEvent on_models_;
};

} // namespace rep
} // namespace prnet

#endif // LIB3DPRNET_REPETIER_CLIENT_HPP
