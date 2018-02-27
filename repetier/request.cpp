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

request::handler_type request::check_ok_flag()
{
    return []( auto const& message ) {
        auto ok { message.find( "ok" ) };
        if ( ok == message.cend() || !ok->is_boolean() ) {
            logger.error( "callback message does not contain valid ok element" );
            throw system_error( make_error_code( prnet_errc::protocol_violation ) );
        }
        if ( !*ok ) {
            throw system_error( make_error_code( prnet_errc::not_ok ) );
        }
    };
}

request::request( string action, callback<> callback )
        : request( internal, move( action ), [callback { move( callback ) }]( auto const& ) { callback(); } ) {}

request::request( internal_t, string&& action, callback< nlohmann::json const& >&& callback )
        : message_ { { "action", move( action ) }, { "data", json::object() } }
        , callback_ { move( callback ) } {}

request::~request() = default;

void request::printer( std::string printer )
{
    message_[ "printer" ] = move( printer );
}

void request::callback_id( size_t id )
{
    message_[ "callback_id" ] = id;
}

void request::add_handler( handler_type handler )
{
    handlers_.push_back( move( handler ) );
}

string request::dump() const
{
    return message_.dump();
}

void request::handle( json const& data ) const
{
    for ( auto& handler : handlers_ ) {
        handler( data );
    }
    callback_( data );
}

} // namespace rep
} // namespace prnet