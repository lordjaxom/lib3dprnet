#ifndef LIB3DPRNET_REPETIER_FRONTEND_HPP
#define LIB3DPRNET_REPETIER_FRONTEND_HPP

#include <memory>
#include <string>
#include <system_error>
#include <vector>

#include <boost/asio/io_context.hpp>
#include <boost/signals2/signal.hpp>

#include "3dprnet/core/config.hpp"
#include "3dprnet/repetier/service.hpp"
#include "3dprnet/repetier/types.hpp"
#include "3dprnet/repetier/upload.hpp"

namespace prnet {
namespace rep {

/**
 * class Frontend
 */

class PRNET_DLL Frontend
        : Service
{
public:
    using Service::Handler;

    using Service::ReconnectEvent;
    using Service::DisconnectEvent;
    using Service::PrintersEvent;
    using Service::GroupsEvent;
    using Service::ModelsEvent;

private:
    struct PrinterData;
    class FrontendImpl;

public:
    Frontend( boost::asio::io_context& context, Endpoint endpoint );
    Frontend( Frontend const& ) = delete;
    ~Frontend();

    using Service::connected;

    void requestPrinters();
    void requestModelGroups( std::string const& slug );
    void requestModels( std::string const& slug );

    using Service::upload;

    using Service::addModelGroup;
    using Service::deleteModelGroup;
    using Service::removeModel;
    using Service::moveModelToGroup;
    using Service::sendCommand;

    void on_reconnect( ReconnectEvent::slot_type const& handler );
    void on_disconnect( DisconnectEvent::slot_type const& handler );
    void on_printers( PrintersEvent::slot_type const& handler );
    void on_groups( GroupsEvent::slot_type const& handler );
    void on_models( ModelsEvent::slot_type const& handler );

private:
    std::unique_ptr< FrontendImpl > impl_;
};

} // namespace rep
} // namespace prnet

#endif //LIB3DPRNET_REPETIER_FRONTEND_HPP
