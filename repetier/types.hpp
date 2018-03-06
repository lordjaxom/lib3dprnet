#ifndef LIB3DPRNET_REPETIER_TYPES_HPP
#define LIB3DPRNET_REPETIER_TYPES_HPP

#include <string>

#include <nlohmann/json_fwd.hpp>

#include "core/config.hpp"
#include "core/string_view.hpp"

namespace prnet {
namespace rep {

/**
 * class settings
 */

class PRNET_DLL settings
{
public:
    settings( std::string host, std::string port, std::string apikey );

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
 * class printer
 */

class PRNET_DLL printer
{
    friend void from_json( nlohmann::json const& src, printer& dst );

public:
    enum state
    {
        disabled, offline, idle, printing
    };

    printer();

    std::string const& name() const { return name_; }
    std::string const& slug() const { return slug_; }
    std::string const& job() const { return job_; }

    state status() const;

private:
    static bool printingJob( std::string const& job );

    bool active_ {};
    std::string name_;
    std::string slug_;
    bool online_ {};
    std::string job_;
};

string_view PRNET_DLL to_string( printer::state state );


/**
 * class group
 */

class PRNET_DLL group
{
    friend void from_json( nlohmann::json const& src, group& dst );

public:
    group();

    std::string const& name() const { return name_; }
    bool defaultGroup() const { return defaultGroup( name_ ); }

private:
    static bool defaultGroup( std::string const& name );

    std::string name_;
};


/**
 * class temperature
 */

class PRNET_DLL temperature
{
    friend void from_json( nlohmann::json const& src, temperature& dst );

public:
    temperature();

    bool heatbed() const { return controller_ == -1; }
    int extruder() const { return controller_; }
    double wanted() const { return wanted_; }
    double actual() const { return actual_; }

    std::string controller_name() const;

private:
    int controller_ {};
    double wanted_ {};
    double actual_ {};
};

} // namespace rep
} // namespace prnet

#endif //LIB3DPRNET_REPETIER_TYPES_HPP
