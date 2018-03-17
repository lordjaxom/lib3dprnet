#ifndef LIB3DPRNET_REPETIER_FRONTEND_HPP
#define LIB3DPRNET_REPETIER_FRONTEND_HPP

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/asio/io_context.hpp>
#include <boost/signals2/signal.hpp>

#include "core/config.hpp"
#include "core/filesystem.hpp"
#include "core/optional.hpp"
#include "service.hpp"
#include "types.hpp"
#include "upload.hpp"

namespace prnet {
namespace rep {

/**
 * class Frontend
 */

class PRNET_DLL Frontend
{
public:
    using Handler = Service::Handler;

    using ReconnectEvent = boost::signals2::signal< void () >;
    using DisconnectEvent = boost::signals2::signal< void ( std::error_code ec ) >;
    using PrintersEvent = boost::signals2::signal< void ( std::vector< Printer > const& printers ) >;
    using GroupsEvent = boost::signals2::signal< void ( std::string const& slug, std::vector< ModelGroup > const& groups ) >;
    using ModelsEvent = boost::signals2::signal< void ( std::string const& slug, std::vector< Model > const& models ) >;

private:
    struct PrinterData
    {
        std::vector< ModelGroup > modelGroups;
        std::vector< Model > models;
    };

public:
    Frontend( boost::asio::io_context& context, Endpoint endpoint );
    Frontend( Frontend const& ) = delete;
    ~Frontend();

    bool connected() const { return service_.connected(); }

    void requestPrinters();
    void requestModelGroups( std::string const& slug );
    void requestModels( std::string const& slug );
    void upload( model_ident ident, filesystem::path path, UploadHandler handler = []( auto ec ) {} );

    void addModelGroup( std::string slug, std::string group, Handler handler = []{}  );
    void deleteModelGroup( std::string slug, std::string group, bool deleteModels, Handler handler = []{}  );
    void removeModel( std::string slug, std::size_t id, Handler handler = []{}  );
    void moveModelToGroup( std::string slug, std::size_t id, std::string group, Handler handler = []{}  );

    void on_reconnect( ReconnectEvent::slot_type const& handler );
    void on_disconnect( DisconnectEvent::slot_type const& handler );
    void on_printers( PrintersEvent::slot_type const& handler );
    void on_groups( GroupsEvent::slot_type const& handler );
    void on_models( ModelsEvent::slot_type const& handler );

private:
    void handlePrinters( std::vector< Printer >&& printers );
    void handleModelGroups( std::string const& slug, std::vector< ModelGroup >&& modelGroups );
    void handleModels( std::string const& slug, std::vector< Model >&& models );

    Service service_;
    optional< std::vector< Printer > > printers_;
    std::unordered_map< std::string, PrinterData > allPrinterData_;
    std::recursive_mutex mutex_;

    ReconnectEvent on_reconnect_;
    DisconnectEvent on_disconnect_;
    PrintersEvent on_printers_;
    GroupsEvent on_groups_;
    ModelsEvent on_models_;
};

} // namespace rep
} // namespace prnet

#endif //LIB3DPRNET_REPETIER_FRONTEND_HPP
