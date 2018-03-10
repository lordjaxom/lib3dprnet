#include <cassert>
#include <algorithm>
#include <utility>

#include <nlohmann/json.hpp>

#include "core/error.hpp"
#include "core/logging.hpp"
#include "client.hpp"
#include "request.hpp"
#include "types.hpp"

using namespace std;
using namespace nlohmann;

namespace prnet {
namespace rep {

static logger logger( "rep::Request" );

/**
 * class Request
 */

Request::Request( string action, ResponseHandler handler )
		: message_ {
            { "action", move( action ) },
            { "data", json::object() }
        }
        , responseHandler_( move( handler ) ) {}

Request::Request( Request&& other ) = default;
Request::~Request() = default;

void Request::printer( string slug )
{
    message_.emplace( "printer", move( slug ) );
}

void Request::cleanup_handler( CleanupHandler handler )
{
    cleanupHandler_ = move( handler );
}

string Request::build( size_t callbackId )
{
    callbackId_ = callbackId;
	message_.emplace( "callback_id", callbackId_ );
    return message_.dump();
}

void Request::handle( json const& data ) const
{
    responseHandler_( data );
    cleanupHandler_();
}

} // namespace rep
} // namespace prnet
