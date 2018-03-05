#ifndef LIB3DPRNET_REPETIER_TYPES_HPP
#define LIB3DPRNET_REPETIER_TYPES_HPP

#include <string>

#include <nlohmann/json_fwd.hpp>

#include "core/config.hpp"

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
    printer();

    bool active() const { return active_; }
    std::string const& name() const { return name_; }
    std::string const& slug() const { return slug_; }
    bool online() const { return online_; }
    std::string const& job() const { return job_; }

private:
    bool active_ {};
    std::string name_;
    std::string slug_;
    bool online_ {};
    std::string job_;
};

void from_json( nlohmann::json const& src, printer& dst );

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

void from_json( nlohmann::json const& src, group& dst );


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

void from_json( nlohmann::json const& src, temperature& dst );

} // namespace rep
} // namespace prnet

#endif //LIB3DPRNET_REPETIER_TYPES_HPP
