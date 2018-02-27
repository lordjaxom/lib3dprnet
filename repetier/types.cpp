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

void from_json( json const& json, printer& printer )
{
    auto active { json.find( "active" ) };
    auto name { json.find( "name" ) };
    auto slug { json.find( "slug" ) };

    if ( active == json.cend() || !active->is_boolean() ||
            name == json.cend() || !name->is_string() ||
            slug == json.cend() || !slug->is_string() ) {
        logger.error( "invalid printer object" );
        throw system_error( make_error_code( prnet_errc::protocol_violation ) );
    }
    printer = { json[ "active" ], json[ "name" ], json[ "slug" ] };
}

/**
 * class group
 */

bool group::defaultGroup( std::string const &name )
{
    return name == "#";
}

group::group( std::string name )
        : name_ { move( name ) } {}

void from_json( json const& json, group& group )
{
    if ( !json.is_string() ) {
        logger.error( "invalid group name" );
    }
}

} // namespace rep
} // namespace prnet