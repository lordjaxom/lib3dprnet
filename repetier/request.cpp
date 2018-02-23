#include <utility>

#include <nlohmann/json.hpp>

#include "core/error.hpp"
#include "request.hpp"

using namespace std;

namespace prnet {
namespace rep {

/**
 * class request
 */

request::handler_type request::check_ok_flag()
{
    return []( auto const& json, auto& ec ) {
        if ( !json[ "data" ][ "ok" ] ) {
            ec = make_error_code( prnet_errc::not_ok );
        }
    };
}

request::request( string action, callback<> callback )
        : request( move( action ), [callback { move( callback ) }]( auto json, auto ec ) { callback( ec ); } ) {}

request::request( string action, callback< nlohmann::json > callback )
        : json_ { { "action", move( action ) } }
        , callback_ { move( callback ) } {}

request::~request() = default;

void request::printer( std::string printer )
{
    json_[ "printer" ] = move( printer );
}

void request::callback_id( size_t id )
{
    json_[ "callback_id" ] = id;
}

void request::add_handler( request::handler_type handler )
{
    handlers_.push_back( move( handler ) );
}

string request::dump() const
{
    return json_.dump();
}

void request::handle( nlohmann::json json ) const
{
    error_code ec;
    find_if( handlers_.cbegin(), handlers_.cend(), [&]( auto const& handler ) {
        handler( json, ec );
        return ec;
    } );
    callback_( move( json ), ec );
}

void request::error( error_code ec ) const
{
    callback_( {}, ec );
}

} // namespace rep
} // namespace prnet