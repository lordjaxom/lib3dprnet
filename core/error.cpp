#include "error.hpp"

using namespace std;

namespace prnet {

/**
 * prnet_category
 */

namespace detail {

string prnet_category::message( int value ) const
{
    switch ( static_cast< errc_t >( value ) ) {
        case example: return "example";
    }
    return "unknown prnet::prnet_category error";
}

} // namespace detail

error_category const& prnet_category()
{
    static detail::prnet_category instance;
    return instance;
}

} // namespace prnet
