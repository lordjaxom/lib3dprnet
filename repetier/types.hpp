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
public:
    printer();
    printer( bool active, std::string name, std::string slug );

    bool active() const { return active_; }
    std::string const& name() const { return name_; }
    std::string const& slug() const { return slug_; }

private:
    bool active_;
    std::string name_;
    std::string slug_;
};

void from_json( nlohmann::json const& json, printer& printer );

/**
 * class group
 */

class PRNET_DLL group
{
public:
    static bool defaultGroup( std::string const& name );

    group();
    group( std::string name );

    std::string const& name() const { return name_; }
    bool defaultGroup() const { return defaultGroup( name_ ); }

private:
    std::string name_;
};

void from_json( nlohmann::json const& json, group& group );

} // namespace rep
} // namespace prnet

#endif //LIB3DPRNET_REPETIER_TYPES_HPP
