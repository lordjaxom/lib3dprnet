#ifndef LIB3DPRNET_CORE_ENCODING_HPP
#define LIB3DPRNET_CORE_ENCODING_HPP

#include <ostream>
#include <sstream>
#include <string>

#include "3dprnet/core/config.hpp"
#include "3dprnet/core/string_view.hpp"

namespace prnet {
namespace enc {

struct PRNET_DLL ToUtf8
{
    friend std::ostream& PRNET_DLL operator<<( std::ostream& os, ToUtf8 const& value );

    ToUtf8( string_view native )
            : native_( native ) {}

private:
    string_view native_;
};

struct PRNET_DLL FromUtf8
{
    friend std::ostream& PRNET_DLL operator<<( std::ostream& os, FromUtf8 const& value );

    FromUtf8( string_view utf8 )
            : utf8_( utf8 ) {}

private:
    string_view utf8_;
};

template< typename To >
std::string convert( std::string const& value )
{
    return static_cast< std::ostringstream& >( std::ostringstream() << ToUtf8( value ) ).str();
}

} // namespace enc
} // namespace prnet

#endif //LIB3DPRNET_CORE_ENCODING_HPP
