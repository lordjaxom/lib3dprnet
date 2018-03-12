#ifndef LIB3DPRNET_REPETIER_TYPES_HPP
#define LIB3DPRNET_REPETIER_TYPES_HPP

#include <chrono>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "core/config.hpp"
#include "core/optional.hpp"
#include "core/string_view.hpp"

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
 * class extruder_config
 */

class PRNET_DLL extruder_config
{
    friend void PRNET_DLL from_json( nlohmann::json const& src, extruder_config& dst );

public:
    int max_temperature() const { return maxTemp_; }

private:
    int maxTemp_ {};
};


/**
 * class heatbed_config
 */

class PRNET_DLL heatbed_config
{
    friend void PRNET_DLL from_json( nlohmann::json const& src, heatbed_config& dst );

public:
    int max_temperature() const { return maxTemp_; }

private:
    int maxTemp_ {};
};


/**
 * class printer_config
 */

class PRNET_DLL printer_config
{
    friend void PRNET_DLL from_json( nlohmann::json const& src, printer_config& dst );

public:
    bool active() const { return active_; }
    std::string const& slug() const { return slug_; }
    std::string const& name() const { return name_; }
    std::string const& firmwareName() const { return firmwareName_; }
    std::vector< extruder_config > const& extruders() const { return extruders_; }
    optional< heatbed_config > const& heatbed() const { return heatbed_; }

private:
    bool active_ {};
    std::string slug_;
    std::string name_;
    std::string firmwareName_;
    std::vector< extruder_config > extruders_;
    optional< heatbed_config > heatbed_;
};


/**
 * class printer
 */

class PRNET_DLL printer
{
    friend void PRNET_DLL from_json( nlohmann::json const& src, printer& dst );

public:
    enum state_t
    {
        disabled, offline, idle, printing
    };

    std::string const& name() const { return name_; }
    std::string const& slug() const { return slug_; }
    std::string const& job() const { return job_; }

    state_t state() const;

private:
    static bool printingJob( std::string const& job );

    bool active_ {};
    std::string name_;
    std::string slug_;
    bool online_ {};
    std::string job_;
};

string_view PRNET_DLL to_string( printer::state_t state );


/**
 * class group
 */

class PRNET_DLL group
{
    friend void PRNET_DLL from_json( nlohmann::json const& src, group& dst );

public:
    std::string const& name() const { return name_; }
    bool defaultGroup() const { return defaultGroup( name_ ); }

private:
    static bool defaultGroup( std::string const& name );

    std::string name_;
};


/**
 * class model
 */

class PRNET_DLL model
{
    friend void PRNET_DLL from_json( nlohmann::json const& src, model& dst );

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
 * class temperature
 */

class PRNET_DLL temperature
{
    friend void PRNET_DLL from_json( nlohmann::json const& src, temperature& dst );

public:
    enum controller_t
    {
        extruder, heatbed
    };

    controller_t controller() const { return controller_; }
    std::size_t controller_index() const { return controllerIndex_; }
    double wanted() const { return wanted_; }
    double actual() const { return actual_; }

    std::string controller_name() const;

private:
    controller_t controller_ {};
    std::size_t controllerIndex_ {};
    double wanted_ {};
    double actual_ {};
};

string_view PRNET_DLL to_string( temperature::controller_t controller );

} // namespace rep
} // namespace prnet

#endif //LIB3DPRNET_REPETIER_TYPES_HPP
