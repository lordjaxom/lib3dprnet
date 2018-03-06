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

bool printer::printingJob( std::string const& job )
{
    return job != "none";
}

printer::printer() = default;

printer::state printer::status() const
{
    return !active_ ? disabled :
           !online_ ? offline :
           !printingJob( job_ ) ? idle :
           printing;
}

void from_json( json const& src, printer& dst )
{
    dst.active_ = src.at( "active" );
    dst.name_ = src.at( "name" );
    dst.slug_ = src.at( "slug" );
    dst.online_ = src.at( "online" ) != 0;
    dst.job_ = src.at( "job" );
}

string_view to_string( printer::state state )
{
    switch ( state ) {
        case printer::disabled: return "disabled";
        case printer::offline: return "offline";
        case printer::idle: return "idle";
        case printer::printing: return "printing";
    }
    throw invalid_argument( "unknown printer::state" );
}

/**
 * class group
 */

bool group::defaultGroup( std::string const &name )
{
    return name == "#";
}

group::group() = default;

void from_json( json const& src, group& dst )
{
    dst.name_ = src;
}


/**
 * class temperature
 */

temperature::temperature() = default;

string temperature::controller_name() const
{
    if ( controller_ == -1 ) {
        return "bed";
    }
    return "e" + std::to_string( controller_ );
}

void from_json( json const &src, temperature& dst )
{
    dst.controller_ = src.at( "id" );
    dst.wanted_ = src.at( "S" );
    dst.actual_ = src.at( "T" );
}

} // namespace rep
} // namespace prnet