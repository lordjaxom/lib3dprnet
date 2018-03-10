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

Endpoint::Endpoint( string host, string port, string apikey )
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
 * class extruder_config
 */

void from_json( nlohmann::json const& src, extruder_config& dst )
{
    dst.maxTemp_ = src.at( "maxTemp" );
}


/**
 * class heatbed_config
 */

void from_json( nlohmann::json const& src, heatbed_config& dst )
{
    dst.maxTemp_ = src.at( "maxTemp" );
}


/**
 * class printer_config
 */

void from_json( nlohmann::json const& src, printer_config& dst )
{
    auto const& general = src.at( "general" );
    dst.active_ = general.at( "active" );
    dst.slug_ = general.at( "slug" );
    dst.name_ = general.at( "name" );
    dst.firmwareName_ = general.at( "firmwareName" );

    dst.extruders_ = src.at( "extruders" ).get< vector< extruder_config > >();

    auto heatedBed = src.find( "heatedBed" );
    dst.heatbed_ = general.at( "heatedBed" ) && heatedBed != src.end() && heatedBed->at( "installed" )
                   ? optional< heatbed_config >( *heatedBed )
                   : nullopt;
}


/**
 * class printer
 */

bool printer::printingJob( std::string const& job )
{
    return job != "none";
}

printer::state_t printer::state() const
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

string_view to_string( printer::state_t state )
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

void from_json( json const& src, group& dst )
{
    dst.name_ = src;
}


/**
 * class model
 */

void from_json( json const& src, model& dst )
{
    dst.id_ = src.at( "id" );
    dst.name_ = src.at( "name" );
    dst.modelGroup_ = src.at( "group" );
    dst.created_ = src.at( "created" ).get< size_t >() / 1000;
    dst.length_ = src.at( "length" );
    dst.layers_ = src.at( "layer" );
    dst.lines_ = src.at( "lines" );
    dst.printTime_ = chrono::milliseconds( static_cast< uint64_t >( src.at( "printTime" ).get< double >() * 1000.0 ) );
}


/**
 * class temperature
 */

string temperature::controller_name() const
{
    switch ( controller_ ) {
        case heatbed: return string( to_string( controller_ ) );
        case extruder: return string( to_string( controller_ ) ) + std::to_string( controllerIndex_ );
    }
    throw invalid_argument( "unknown temperature::controller" );
}

void from_json( json const &src, temperature& dst )
{
    int index = src.at( "id" );
    if ( index >= 0 ) {
        dst.controller_ = temperature::extruder;
        dst.controllerIndex_ = static_cast< size_t >( index );
    } else if ( index == -1 ) {
        dst.controller_ = temperature::heatbed;
        dst.controllerIndex_ = 0;
    } else {
        throw invalid_argument( "unknown temperature controller id " + std::to_string( index ) );
    }
    dst.wanted_ = src.at( "S" );
    dst.actual_ = src.at( "T" );
}

string_view to_string( temperature::controller_t controller )
{
    switch ( controller ) {
        case temperature::extruder: return "extruder";
        case temperature::heatbed: return "heatbed";
    }
    throw invalid_argument( "unknown temperature::controller" );
}

} // namespace rep
} // namespace prnet