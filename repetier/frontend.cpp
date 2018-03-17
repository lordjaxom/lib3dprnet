#include <mutex>
#include <unordered_map>
#include <utility>

#include "core/optional.hpp"
#include "frontend.hpp"
#include "service.hpp"
#include "types.hpp"

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
    
class Frontend::Impl
{
public:
    Impl( boost::asio::io_context& context, Endpoint&& endpoint )
            : service_( context, move( endpoint ) )
    {
        service_.on_disconnect( [this]( auto ec ) { on_disconnect_( ec ); } );
        service_.on_printers( [this]( auto printers ) { this->handlePrinters( move( printers ) ); } );
        service_.on_groups( [this]( auto slug, auto modelGroups ) { this->handleModelGroups( slug, move( modelGroups ) ); } );
        service_.on_models( [this]( auto slug, auto models ) { this->handleModels( slug, move( models ) ); } );
        service_.request_printers();
    }

    bool connected() const { return service_.connected(); }

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
    
    void addModelGroup( string&& slug, string&& group, Handler&& handler )
    {
        service_.add_model_group( move( slug ), move( group ), move( handler ) );
    }

    void deleteModelGroup( string&& slug, string&& group, bool deleteModels, Handler&& handler )
    {
        service_.delete_model_group( move( slug ), move( group ), deleteModels, move( handler ) );
    }

    void removeModel( string&& slug, size_t id, Handler&& handler )
    {
        service_.remove_model( move( slug ), id, move( handler ) );
    }

    void moveModelToGroup( string&& slug, size_t id, string&& group, Handler&& handler )
    {
        service_.move_model_to_group( move( slug ), id, move( group ), move( handler ) );
    }

    void upload( model_ident&& ident, filesystem::path&& path, UploadHandler&& handler )
    {
        service_.upload( move( ident ), move( path ), move( handler ) );
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

Frontend::Frontend( boost::asio::io_context& context, Endpoint endpoint )
        : impl_( make_unique< Impl >( context, move( endpoint ) ) ) {}

Frontend::~Frontend() = default;

bool Frontend::connected() const { return impl_->connected(); }

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
    
void Frontend::addModelGroup( string slug, string group, Handler handler )
{
    impl_->addModelGroup( move( slug ), move( group ), move( handler ) );
}

void Frontend::deleteModelGroup( string slug, string group, bool deleteModels, Handler handler )
{
    impl_->deleteModelGroup( move( slug ), move( group ), deleteModels, move( handler ) );
}

void Frontend::removeModel( string slug, size_t id, Handler handler )
{
    impl_->removeModel( move( slug ), id, move( handler ) );
}

void Frontend::moveModelToGroup( string slug, size_t id, string group, Handler handler )
{
    impl_->moveModelToGroup( move( slug ), id, move( group ), move( handler ) );
}

void Frontend::upload( model_ident ident, filesystem::path path, UploadHandler handler )
{
    impl_->upload( move( ident ), move( path ), move( handler ) );
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