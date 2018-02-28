#include <utility>

#include <nlohmann/json.hpp>
#include <core/error.hpp>

#include "core/error.hpp"
#include "core/logging.hpp"
#include "types.hpp"

using namespace std;
using namespace nlohmann;

namespace prnet {
namespace rep {

static logger logger( "rep::types" );

/**
 * class settings
 */

settings::settings( string host, string port, string apikey )
        : host_ { move( host ) }
        , port_ { move( port ) }
        , apikey_ { move( apikey ) } {}


/**
 * class model_ident
 */

model_ident::model_ident( std::string printer, std::string name, std::string group )
        : printer_ { move( printer ) }
        , name_ { move( name ) }
        , group_ { move( group ) } {}

/**
 * class printer
 */

printer::printer() = default;

printer::printer( bool active, std::string name, std::string slug )
        : active_ { active }
        , name_ { move( name ) }
        , slug_ { move( slug ) } {}

void from_json( json const& data, printer& printer )
{
    printer = { data[ "active" ], data[ "name" ], data[ "slug" ] };
}

/**
 * class group
 */

bool group::defaultGroup( std::string const &name )
{
    return name == "#";
}

group::group() = default;

group::group( std::string name )
        : name_ { move( name ) } {}

void from_json( json const& data, group& group )
{
    group = { data.get< string >() };
}

} // namespace rep
} // namespace prnet