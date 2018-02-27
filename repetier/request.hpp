#ifndef LIB3DPRNET_REQUEST_HPP
#define LIB3DPRNET_REQUEST_HPP

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

#include "core/config.hpp"
#include "forward.hpp"

namespace prnet {
namespace rep {

/**
 * class request
 */

class PRNET_DLL request
{
public:
    static callback< nlohmann::json& > check_ok_flag();

    request( std::string action, callback< nlohmann::json > cb );
    request( std::string action, callback<> cb );
    ~request();

    void printer( std::string printer );
    void callback_id( size_t id );

    template< typename T >
    void set( std::string key, T&& value )
    {
        message_[ "data" ].emplace( std::move( key ), std::forward< T >( value ) );
    }

    void add_handler( callback< nlohmann::json& > handler );

    std::string dump() const;
    void handle( nlohmann::json data ) const;

private:
    nlohmann::json message_;
    std::vector< callback< nlohmann::json& > > handlers_;
    callback< nlohmann::json > callback_;
};

} // namespace rep
} // namespace prnet

#endif //LIB3DPRNET_REQUEST_HPP
