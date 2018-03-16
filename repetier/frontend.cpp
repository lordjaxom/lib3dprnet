#include <algorithm>
#include <iterator>
#include <unordered_set>
#include <utility>

#include "frontend.hpp"

using namespace std;

namespace prnet {
namespace rep {

namespace detail {

using Lock = lock_guard< recursive_mutex >;

} // namespace detail


/**
 * class Frontend
 */

Frontend::Frontend( boost::asio::io_context& context, Endpoint endpoint )
        : service_( context, move( endpoint ) )
{
    service_.on_printers( [this]( auto printers ) { this->handlePrinters( move( printers ) ); } );
    service_.on_groups( [this]( auto slug, auto modelGroups ) { this->handleModelGroups( slug, move( modelGroups ) ); } );
    service_.on_models( [this]( auto slug, auto models ) { this->handleModels( slug, move( models ) ); } );
    service_.request_printers();
}

Frontend::~Frontend() = default;

void Frontend::requestPrinters()
{
    detail::Lock lock( mutex_ );

    if ( printers_ != nullopt ) {
        on_printers_( *printers_ );
    }
}

void Frontend::requestModelGroups( string const& slug )
{
    detail::Lock lock( mutex_ );

    auto printerData = allPrinterData_.find( slug );
    if ( printerData != allPrinterData_.end() ) {
        on_groups_( slug, printerData->second.modelGroups );
    }
}

void Frontend::requestModels( string const& slug )
{
    detail::Lock lock( mutex_ );

    auto printerData = allPrinterData_.find( slug );
    if ( printerData != allPrinterData_.end() ) {
        on_models_( slug, printerData->second.models );
    }
}

void Frontend::on_reconnect( ReconnectEvent::slot_type const& handler )
{
    on_reconnect_.connect( handler );
}

void Frontend::on_disconnect( DisconnectEvent::slot_type const& handler )
{
    on_disconnect_.connect( handler );
}

void Frontend::on_printers( PrintersEvent::slot_type const& handler )
{
    on_printers_.connect( handler );
}

void Frontend::on_groups( GroupsEvent::slot_type const& handler )
{
    on_groups_.connect( handler );
}

void Frontend::on_models( ModelsEvent::slot_type const& handler )
{
    on_models_.connect( handler );
}

void Frontend::handlePrinters( std::vector< Printer >&& printers )
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

void Frontend::handleModelGroups( string const& slug, vector< ModelGroup >&& modelGroups )
{
    detail::Lock lock( mutex_ );

    auto& printerData = allPrinterData_.at( slug );
    printerData.modelGroups = move( modelGroups );
    on_groups_( slug, printerData.modelGroups );
}

void Frontend::handleModels( string const& slug, vector< Model >&& models )
{
    detail::Lock lock( mutex_ );

    auto& printerData = allPrinterData_.at( slug );
    printerData.models = move( models );
    on_models_( slug, printerData.models );
}

} // namespace rep
} // namespace prnet