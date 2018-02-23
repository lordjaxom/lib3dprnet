#ifndef LIB3DPRNET_REQUEST_HPP
#define LIB3DPRNET_REQUEST_HPP

#include <functional>
#include <memory>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

#include "types.hpp"

namespace prnet {
namespace rep {

/**
 * class request
 */

class request
{
public:
    using handler_type = std::function< void ( nlohmann::json, std::error_code& ) >;

    static handler_type check_ok_flag();

    request( std::string action, callback<> callback );
    ~request();

    void printer( std::string printer );
    void callback_id( size_t id );

    template< typename T >
    void set( std::string key, T&& value )
    {
        json_[ "data" ].emplace( std::move( key ), std::forward< T >( value ) );
    }

    void add_handler( handler_type handler );

    std::string dump() const;
    void handle( nlohmann::json json ) const;
    void error( std::error_code ec ) const;

private:
    request( std::string action, callback< nlohmann::json > callback );

    nlohmann::json json_;
    std::vector< handler_type > handlers_;
    callback< nlohmann::json > callback_;
};

} // namespace rep
} // namespace prnet

#endif //LIB3DPRNET_REQUEST_HPP
