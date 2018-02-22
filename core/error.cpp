#include "error.hpp"

using namespace std;

namespace prnet {

/**
 * function prnet_category
 */

namespace detail {

string prnet_category::message( int value ) const
{
    switch ( static_cast< errc >( value ) ) {
        case errc::server_error: return "server error";
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

error_code make_error_code( errc e )
{
    return error_code( static_cast< int >( e ), prnet_category() );
}

} // namespace prnet
