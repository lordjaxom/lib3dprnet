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

callback< json& > request::check_ok_flag()
{
    return []( auto& data ) {
        if ( !data[ "ok" ] ) {
            throw system_error( make_error_code( prnet_errc::not_ok ) );
        }
    };
}

request::request( string action, callback< json > cb )
        : message_ { { "action", move( action ) }, { "data", json::object() } }
        , callback_ { move( cb ) } {}

request::request( string action, callback<> cb )
        : request { move( action ), [cb { move( cb ) }]( auto ) { cb(); } } {}

request::~request() = default;

void request::printer( string printer )
{
    message_[ "printer" ] = move( printer );
}

void request::callback_id( size_t id )
{
    message_[ "callback_id" ] = id;
}

void request::add_handler( callback< json& > handler )
{
    handlers_.push_back( move( handler ) );
}

string request::dump() const
{
    return message_.dump();
}

void request::handle( json data ) const
{
    for_each( handlers_.cbegin(), handlers_.cend(), [&]( auto const& cb ) { cb( data ); } );
    callback_( move( data ) );
}

} // namespace rep
} // namespace prnet