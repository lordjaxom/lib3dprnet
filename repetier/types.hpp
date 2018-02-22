#ifndef LIB3DPRNET_REPETIER_TYPES_HPP
#define LIB3DPRNET_REPETIER_TYPES_HPP

#include <string>

namespace prnet {
namespace rep {

/**
 * class settings
 */

class settings
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

class model_ident
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

} // namespace rep
} // namespace prnet

#endif //LIB3DPRNET_REPETIER_TYPES_HPP
