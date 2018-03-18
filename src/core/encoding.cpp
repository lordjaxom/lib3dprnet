#include <algorithm>
#include <iterator>

#include <utf8.h>

#include "3dprnet/core/encoding.hpp"

using namespace std;

namespace prnet {
namespace enc {

ostream& operator<<( ostream& os, ToUtf8 const& value )
{
    for_each( value.native_.begin(), value.native_.end(), [out = ostream_iterator< uint8_t >( os )]( uint8_t ch ) {
        utf8::unchecked::append( static_cast< uint32_t >( ch ), out );
    } );
}

ostream& operator<<( ostream& os, FromUtf8 const& value )
{
    for ( auto it = value.utf8_.begin() ; it != value.utf8_.end() ; ) {
        uint32_t cp = utf8::unchecked::next( it );
        os << ( cp < 256 ? static_cast< char >( static_cast< uint8_t >( cp ) ) : '_' );
    }
}

} // namespace enc
} // namespace prnet
