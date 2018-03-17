#ifndef LIB3DPRNET_REPETIER_CLIENT_HPP
#define LIB3DPRNET_REPETIER_CLIENT_HPP

#include <functional>
#include <memory>
#include <string>
#include <system_error>

#include <boost/asio/io_context.hpp>
#include <boost/signals2/signal.hpp>
#include <nlohmann/json_fwd.hpp>

#include "core/config.hpp"
#include "forward.hpp"
#include "upload.hpp"

namespace prnet {
namespace rep {

/**
 * class Service
 */

class PRNET_DLL Service
{
public:
	using CallbackHandler = std::function< void ( nlohmann::json const& data ) >;

    using Handler = std::function< void () >;

	using ReconnectEvent = boost::signals2::signal< void () >;
    using DisconnectEvent = boost::signals2::signal< void ( std::error_code ec ) >;
    using TemperatureEvent = boost::signals2::signal< void ( std::string slug, Temperature temp ) >;
    using PrintersEvent = boost::signals2::signal< void ( std::vector< Printer > printers ) >;
    using ConfigEvent = boost::signals2::signal< void ( std::string slug, PrinterConfig config ) >;
    using GroupsEvent = boost::signals2::signal< void ( std::string slug, std::vector< ModelGroup > groups ) >;
    using ModelsEvent = boost::signals2::signal< void ( std::string slug, std::vector< Model > models ) >;

private:
	struct Action;
    class Impl;

public:
    Service( boost::asio::io_context& context, Endpoint endpoint );
    Service( Service const& ) = delete;
    ~Service();

    bool connected() const;

    void request_printers();
    void request_config( std::string slug );
    void request_groups( std::string slug );
    void request_models( std::string slug );

	void add_model_group( std::string slug, std::string group, Handler handler = []{} );
    void delete_model_group( std::string slug, std::string group, bool deleteModels, Handler handler = []{} );
    void remove_model( std::string slug, std::size_t id, Handler handler = []{} );
    void move_model_to_group( std::string slug, std::size_t id, std::string group, Handler handler = []{} );
	void upload( model_ident ident, filesystem::path path, UploadHandler handler = []( auto ec ) {} );

    void on_reconnect( ReconnectEvent::slot_type const& handler );
    void on_disconnect( DisconnectEvent::slot_type const& handler );
    void on_temperature( TemperatureEvent::slot_type const& handler );
    void on_printers( PrintersEvent::slot_type const& handler );
    void on_config( ConfigEvent::slot_type const& handler );
    void on_groups( GroupsEvent::slot_type const& handler );
    void on_models( ModelsEvent::slot_type const& handler );

private:
    std::unique_ptr< Impl > impl_;
};

} // namespace rep
} // namespace prnet

#endif // LIB3DPRNET_REPETIER_CLIENT_HPP
