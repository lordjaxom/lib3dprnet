#include <utility>

#include "repetier/types.hpp"

using namespace std;

namespace prnet {
namespace rep {

/**
 * class settings
 */

settings::settings( string host, string port, string apikey )
        : host_( move( host ) )
        , port_( move( port ) )
        , apikey_( move( apikey ) ) {}


/**
 * class model_ident
 */

model_ident::model_ident( std::string printer, std::string name, std::string group )
        : printer_( move( printer ) )
        , name_( move( name ) )
        , group_( move( group ) ) {}

} // namespace rep
} // namespace prnet