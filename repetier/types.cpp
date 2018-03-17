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

static Logger logger( "rep::types" );

/**
 * class Endpoint
 */

Endpoint::Endpoint() = default;

Endpoint::Endpoint( string host, string port, string apikey )
        : host_ { move( host ) }
        , port_ { move( port ) }
        , apikey_ { move( apikey ) } {}

void from_json( json const& src, Endpoint& dst )
{
    dst.host_ = src.at( "host" );
    dst.port_ = src.at( "port" );
    dst.apikey_ = src.at( "apikey" );
}


/**
 * class model_ident
 */

model_ident::model_ident( string printer, string group, string name )
        : printer_ { move( printer ) }
        , name_ { move( name ) }
        , group_ { move( group ) } {}


/**
 * class ExtruderConfig
 */

void from_json( json const& src, ExtruderConfig& dst )
{
    dst.maxTemp_ = src.at( "maxTemp" );
}


/**
 * class HeatbedConfig
 */

void from_json( json const& src, HeatbedConfig& dst )
{
    dst.maxTemp_ = src.at( "maxTemp" );
}


/**
 * class PrinterConfig
 */

void from_json( json const& src, PrinterConfig& dst )
{
    auto const& general = src.at( "general" );
    dst.active_ = general.at( "active" );
    dst.slug_ = general.at( "slug" );
    dst.name_ = general.at( "name" );
    dst.firmwareName_ = general.at( "firmwareName" );

    dst.extruders_ = src.at( "extruders" ).get< vector< ExtruderConfig > >();

    auto heatedBed = src.find( "heatedBed" );
    dst.heatbed_ = general.at( "heatedBed" ) && heatedBed != src.end() && heatedBed->at( "installed" )
                   ? optional< HeatbedConfig >( *heatedBed )
                   : nullopt;
}


/**
 * class Printer
 */

bool Printer::printingJob( string const& job )
{
    return job != "none";
}

Printer::State Printer::state() const
{
    return !active_ ? disabled :
           !online_ ? offline :
           !printingJob( job_ ) ? idle :
           printing;
}

void from_json( json const& src, Printer& dst )
{
    dst.active_ = src.at( "active" );
    dst.name_ = src.at( "name" );
    dst.slug_ = src.at( "slug" );
    dst.online_ = src.at( "online" ) != 0;
    dst.job_ = src.at( "job" );
}

string_view to_string( Printer::State state )
{
    switch ( state ) {
        case Printer::disabled: return "disabled";
        case Printer::offline: return "offline";
        case Printer::idle: return "idle";
        case Printer::printing: return "printing";
    }
    throw invalid_argument( "unknown Printer::state" );
}

/**
 * class ModelGroup
 */

bool ModelGroup::defaultGroup( string const &name )
{
    return name == "#";
}

void from_json( json const& src, ModelGroup& dst )
{
    dst.name_ = src;
}


/**
 * class Model
 */

void from_json( json const& src, Model& dst )
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
 * class Temperature
 */

string Temperature::controller_name() const
{
    switch ( controller_ ) {
        case heatbed: return string( to_string( controller_ ) );
        case extruder: return string( to_string( controller_ ) ) + std::to_string( controllerIndex_ );
    }
    throw invalid_argument( "unknown Temperature::controller" );
}

void from_json( json const &src, Temperature& dst )
{
    int index = src.at( "id" );
    if ( index >= 0 ) {
        dst.controller_ = Temperature::extruder;
        dst.controllerIndex_ = static_cast< size_t >( index );
    } else if ( index == -1 ) {
        dst.controller_ = Temperature::heatbed;
        dst.controllerIndex_ = 0;
    } else {
        throw invalid_argument( "unknown temperature controller id " + std::to_string( index ) );
    }
    dst.wanted_ = src.at( "S" );
    dst.actual_ = src.at( "T" );
}

string_view to_string( Temperature::Controller controller )
{
    switch ( controller ) {
        case Temperature::extruder: return "extruder";
        case Temperature::heatbed: return "heatbed";
    }
    throw invalid_argument( "unknown Temperature::controller" );
}

} // namespace rep
} // namespace prnet