#include <mutex>
#include <unordered_map>
#include <utility>

#include "3dprnet/core/optional.hpp"
#include "3dprnet/repetier/frontend.hpp"
#include "3dprnet/repetier/service.hpp"
#include "3dprnet/repetier/types.hpp"

using namespace std;

namespace prnet {
namespace rep {

namespace detail {

using Lock = lock_guard< recursive_mutex >;

} // namespace detail


/**
 * class Frontend
 */

struct Frontend::PrinterData
{
    std::vector< ModelGroup > modelGroups;
    std::vector< Model > models;
};
    
class Frontend::FrontendImpl
{
public:
    FrontendImpl( Service& service )
            : service_( service )
    {
        service_.on_disconnect( [this]( auto ec ) { on_disconnect_( ec ); } );
        service_.on_printers( [this]( auto printers ) { this->handlePrinters( move( printers ) ); } );
        service_.on_groups( [this]( auto slug, auto modelGroups ) { this->handleModelGroups( slug, move( modelGroups ) ); } );
        service_.on_models( [this]( auto slug, auto models ) { this->handleModels( slug, move( models ) ); } );
        service_.request_printers();
    }

    void requestPrinters()
    {
        detail::Lock lock( mutex_ );

        if ( printers_ != nullopt ) {
            on_printers_( *printers_ );
        }
    }

    void requestModelGroups( std::string const& slug )
    {
        detail::Lock lock( mutex_ );

        auto printerData = allPrinterData_.find( slug );
        if ( printerData != allPrinterData_.end() ) {
            on_groups_( slug, printerData->second.modelGroups );
        }
    }

    void requestModels( std::string const& slug )
    {
        detail::Lock lock( mutex_ );

        auto printerData = allPrinterData_.find( slug );
        if ( printerData != allPrinterData_.end() ) {
            on_models_( slug, printerData->second.models );
        }
    }

    void on_reconnect( ReconnectEvent::slot_type const& handler )
    {
        on_reconnect_.connect( handler );
    }

    void on_disconnect( DisconnectEvent::slot_type const& handler )
    {
        on_disconnect_.connect( handler );
    }

    void on_printers( PrintersEvent::slot_type const& handler )
    {
        on_printers_.connect( handler );
    }

    void on_groups( GroupsEvent::slot_type const& handler )
    {
        on_groups_.connect( handler );
    }

    void on_models( ModelsEvent::slot_type const& handler )
    {
        on_models_.connect( handler );
    }

private:
    void handlePrinters( std::vector< Printer >&& printers )
    {
        detail::Lock lock( mutex_ );

        unordered_map< string, PrinterData > allPrinterData;
        for ( auto const& printer : printers ) {
            auto printerData = allPrinterData_.find( printer.slug() );
            if ( printerData != allPrinterData_.end() ) {
                allPrinterData.insert( move( *printerData ) );
            } else {
                allPrinterData.emplace( printer.slug(), PrinterData() );
            }

            service_.request_groups( printer.slug() );
            service_.request_models( printer.slug() );
        }

        allPrinterData_ = move( allPrinterData );
        printers_ = move( printers );
        on_printers_( *printers_ );
    }

    void handleModelGroups( string const& slug, vector< ModelGroup >&& modelGroups )
    {
        detail::Lock lock( mutex_ );

        auto& printerData = allPrinterData_.at( slug );
        printerData.modelGroups = move( modelGroups );
        on_groups_( slug, printerData.modelGroups );
    }

    void handleModels( string const& slug, vector< Model >&& models )
    {
        detail::Lock lock( mutex_ );

        auto& printerData = allPrinterData_.at( slug );
        printerData.models = move( models );
        on_models_( slug, printerData.models );
    }

    Service& service_;
    optional< std::vector< Printer > > printers_;
    std::unordered_map< std::string, PrinterData > allPrinterData_;
    std::recursive_mutex mutex_;

    ReconnectEvent on_reconnect_;
    DisconnectEvent on_disconnect_;
    PrintersEvent on_printers_;
    GroupsEvent on_groups_;
    ModelsEvent on_models_;
};

Frontend::Frontend( boost::asio::io_context& context, Endpoint endpoint )
        : Service( context, move( endpoint ) )
        , impl_( make_unique< FrontendImpl >( static_cast< Service& >( *this ) ) ) {}

Frontend::~Frontend() = default;

void Frontend::requestPrinters()
{
    impl_->requestPrinters();
}
    
void Frontend::requestModelGroups( string const& slug )
{
    impl_->requestModelGroups( slug );
}
    
void Frontend::requestModels( string const& slug )
{
    impl_->requestModels( slug );
}

void Frontend::on_reconnect( ReconnectEvent::slot_type const& handler )
{
    impl_->on_reconnect( handler );
}

void Frontend::on_disconnect( DisconnectEvent::slot_type const& handler )
{
    impl_->on_disconnect( handler );
}

void Frontend::on_printers( PrintersEvent::slot_type const& handler )
{
    impl_->on_printers( handler );
}

void Frontend::on_groups( GroupsEvent::slot_type const& handler )
{
    impl_->on_groups( handler );
}

void Frontend::on_models( ModelsEvent::slot_type const& handler )
{
    impl_->on_models( handler );
}

} // namespace rep
} // namespace prnet