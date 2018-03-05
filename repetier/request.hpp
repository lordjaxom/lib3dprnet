#ifndef LIB3DPRNET_REQUEST_HPP
#define LIB3DPRNET_REQUEST_HPP

#include <functional>
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
    using handler = std::function< void ( nlohmann::json& ) >;

    static handler check_ok_flag();
    static handler resolve_element( std::string key );

    explicit request( std::string action );
    ~request();

    void printer( std::string printer );
    void callback_id( size_t id );

    template< typename T >
    void set( std::string key, T&& value )
    {
        message_[ "data" ].emplace( std::move( key ), std::forward< T >( value ) );
    }

    void add_handler( handler h );

    std::string dump() const;
    void handle( nlohmann::json data ) const;

private:
    nlohmann::json message_;
    std::vector< handler > handlers_;
};

} // namespace rep
} // namespace prnet

#endif //LIB3DPRNET_REQUEST_HPP
