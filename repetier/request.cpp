#include <algorithm>
#include <utility>

#include <nlohmann/json.hpp>

#include "core/error.hpp"
#include "core/logging.hpp"
#include "request.hpp"
#include "types.hpp"

using namespace std;
using namespace nlohmann;

namespace prnet {
namespace rep {

static logger logger( "rep::request" );

/**
 * class request
 */

request::handler request::check_ok_flag()
{
    return []( auto& data ) {
        if ( !data.at( "ok" ) ) {
            throw system_error( make_error_code( prnet_errc::not_ok ) );
        }
    };
}

request::handler request::resolve_element( string key )
{
    return [key { move( key ) }]( auto& data ) {
        data = data.at( key );
    };
}

request::request( string action )
        : message_ { { "action", move( action ) }, { "data", json::object() } } {}

request::~request() = default;

void request::printer( string printer )
{
    message_[ "printer" ] = move( printer );
}

void request::callback_id( size_t id )
{
    message_[ "callback_id" ] = id;
}

void request::add_handler( handler h )
{
    handlers_.push_back( move( h ) );
}

string request::dump() const
{
    return message_.dump();
}

void request::handle( json data ) const
{
    for_each( handlers_.cbegin(), handlers_.cend(), [&data]( auto const& h ) { h( data ); } );
}

} // namespace rep
} // namespace prnet