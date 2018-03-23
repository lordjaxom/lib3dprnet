#ifndef LIB3DPRNET_REPETIER_TYPES_HPP
#define LIB3DPRNET_REPETIER_TYPES_HPP

#include <chrono>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "3dprnet/core/config.hpp"
#include "3dprnet/core/optional.hpp"
#include "3dprnet/core/string_view.hpp"

namespace prnet {
namespace rep {

/**
 * class Endpoint
 */

class PRNET_DLL Endpoint
{
    friend void PRNET_DLL from_json( nlohmann::json const& src, Endpoint& dst );

public:
    Endpoint();
    Endpoint( std::string host, std::string port, std::string apikey );

    std::string const& host() const { return host_; }
    std::string const& port() const { return port_; }
    std::string const& apikey() const { return apikey_; }

private:
    std::string host_;
    std::string port_;
    std::string apikey_;
};


/**
 * class model_ident
 */

class PRNET_DLL model_ident
{
public:
    model_ident( std::string printer, std::string group, std::string name );

    std::string const& printer() const { return printer_; }
    std::string const& group() const { return group_; }
    std::string const& name() const { return name_; }

private:
    std::string printer_;
    std::string group_;
    std::string name_;
};


/**
 * class ExtruderConfig
 */

class PRNET_DLL ExtruderConfig
{
    friend void PRNET_DLL from_json( nlohmann::json const& src, ExtruderConfig& dst );

public:
    int max_temperature() const { return maxTemp_; }

private:
    int maxTemp_ {};
};


/**
 * class HeatbedConfig
 */

class PRNET_DLL HeatbedConfig
{
    friend void PRNET_DLL from_json( nlohmann::json const& src, HeatbedConfig& dst );

public:
    int max_temperature() const { return maxTemp_; }

private:
    int maxTemp_ {};
};


/**
 * class PrinterConfig
 */

class PRNET_DLL PrinterConfig
{
    friend void PRNET_DLL from_json( nlohmann::json const& src, PrinterConfig& dst );

public:
    bool active() const { return active_; }
    std::string const& slug() const { return slug_; }
    std::string const& name() const { return name_; }
    std::string const& firmwareName() const { return firmwareName_; }
    std::vector< ExtruderConfig > const& extruders() const { return extruders_; }
    optional< HeatbedConfig > const& heatbed() const { return heatbed_; }

private:
    bool active_ {};
    std::string slug_;
    std::string name_;
    std::string firmwareName_;
    std::vector< ExtruderConfig > extruders_;
    optional< HeatbedConfig > heatbed_;
};


/**
 * class Printer
 */

class PRNET_DLL Printer
{
    friend void PRNET_DLL from_json( nlohmann::json const& src, Printer& dst );

public:
    enum State
    {
        disabled, offline, idle, printing
    };

    bool active() const { return active_; }
    std::string const& name() const { return name_; }
    std::string const& slug() const { return slug_; }
    bool online() const { return online_; }
    std::string const& job() const { return job_; }

    State state() const;

private:
    static bool printingJob( std::string const& job );

    bool active_ {};
    std::string name_;
    std::string slug_;
    bool online_ {};
    std::string job_;
};

string_view PRNET_DLL to_string( Printer::State state );


/**
 * class ModelGroup
 */

class PRNET_DLL ModelGroup
{
    friend void PRNET_DLL from_json( nlohmann::json const& src, ModelGroup& dst );

public:
    std::string const& name() const { return name_; }
    bool defaultGroup() const { return defaultGroup( name_ ); }

private:
    static bool defaultGroup( std::string const& name );

    std::string name_;
};


/**
 * class Model
 */

class PRNET_DLL Model
{
    friend void PRNET_DLL from_json( nlohmann::json const& src, Model& dst );

public:
    std::size_t id() const { return id_; }
    std::string const& name() const { return name_; }
    std::string const& modelGroup() const { return modelGroup_; }
    std::time_t const& created() const { return created_; }
    std::size_t length() const { return length_; }
    std::size_t layers() const { return layers_; }
    std::size_t lines() const { return lines_; }
    std::chrono::microseconds printTime() const { return printTime_; }

private:
    std::size_t id_ {};
    std::string name_;
    std::string modelGroup_;
    std::time_t created_ {};
    std::size_t length_ {};
    std::size_t layers_ {};
    std::size_t lines_ {};
    std::chrono::microseconds printTime_;
};


/**
 * class Temperature
 */

class PRNET_DLL Temperature
{
    friend void PRNET_DLL from_json( nlohmann::json const& src, Temperature& dst );

public:
    enum Controller
    {
        extruder, heatbed
    };

    Controller controller() const { return controller_; }
    std::size_t controller_index() const { return controllerIndex_; }
    double wanted() const { return wanted_; }
    double actual() const { return actual_; }

    std::string controller_name() const;

private:
    Controller controller_ {};
    std::size_t controllerIndex_ {};
    double wanted_ {};
    double actual_ {};
};

string_view PRNET_DLL to_string( Temperature::Controller controller );

} // namespace rep
} // namespace prnet

#endif //LIB3DPRNET_REPETIER_TYPES_HPP
