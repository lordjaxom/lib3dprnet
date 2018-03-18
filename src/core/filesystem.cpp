#include <locale>
#include <type_traits>
#include <vector>

#include "3dprnet/core/filesystem.hpp"

using namespace std;

namespace prnet {
namespace filesystem {

string native_path( path const& path )
{
#if defined( WIN32 ) 
    auto native = path.native();
    vector< char > result( native.size() );
    use_facet< ctype< wchar_t > >( locale( "" ) ).narrow( &native[ 0 ], &native[ native.size() ], '_', &result[ 0 ] );
    return { result.begin(), result.end() };
#else
	return path.native();
#endif
}

} // namespace filesystem
} // namespace prnet
