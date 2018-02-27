#include "error.hpp"

using namespace std;

namespace prnet {

/**
 * function prnet_category
 */

namespace detail {

string prnet_category::message( int value ) const
{
    switch ( static_cast< prnet_errc >( value ) ) {
        case prnet_errc::not_ok: return "server returned nok";
        case prnet_errc::exception: return "exception caught";
        case prnet_errc::server_error: return "server side error";
        case prnet_errc::protocol_violation: return "protocol violation";
    }
    return "unknown prnet::prnet_category error";
}

} // namespace detail

error_category const& prnet_category()
{
    static detail::prnet_category instance;
    return instance;
}


/**
 * function make_error_code
 */

error_code make_error_code( prnet_errc e )
{
    return error_code( static_cast< int >( e ), prnet_category() );
}

} // namespace prnet
