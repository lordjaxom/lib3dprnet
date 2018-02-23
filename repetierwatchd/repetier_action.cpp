#include "repetier_action.hpp"
#include "repetier_socket.hpp"

using namespace std;

namespace rep {

    RepetierAction::Handler RepetierAction::checkOk()
    {
        return []( auto const& data, auto& ec ) {
            if ( !data[ "ok" ] ) {
                ec = boost::system::errc::make_error_code( boost::system::errc::invalid_argument );
            }
        };
    }

    RepetierAction::RepetierAction( size_t callbackId, string&& request, Handler&& handler ):
            callbackId_( callbackId ),
            request_( move( request ) ),
            handler_( move( handler ) )
    {
    }

    void RepetierAction::handle( nlohmann::json const& data, boost::system::error_code& ec )
    {
        if ( handler_ ) {
            handler_( data, ec );
        }
    }


    RepetierActionBuilder::RepetierActionBuilder( string action )
    {
        json_[ "action" ] = move( action );
    }

    RepetierActionBuilder::RepetierActionBuilder( std::string action, std::string printer ):
            RepetierActionBuilder( std::move( action ) )
    {
        json_[ "printer" ] = std::move( printer );
    }

    void RepetierActionBuilder::send( RepetierSocket& socket )
    {
        auto callbackId = socket.nextCallbackId();
        json_[ "callback_id" ] = callbackId;
        socket.send( std::make_unique< RepetierAction >( callbackId, json_.dump(), std::move( handler_ )  ) );
    }

} // namespace rep