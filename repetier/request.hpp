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
    using json_handler = std::function< void ( nlohmann::json& ) >;
    using void_handler = std::function< void () >;

    static json_handler check_ok_flag();
    static json_handler resolve_element( char const* key );

    request( client& cl, std::string action );
    request( request const& ) = delete;
    request( request&& );
    ~request();

    std::size_t callbackId() const { return callbackId_; }

    void printer( std::string printer );

    template< typename T >
    void set( std::string key, T&& value )
    {
        message_[ "data" ].emplace( std::move( key ), std::forward< T >( value ) );
    }

    void add_handler( json_handler h );
    void add_handler( void_handler h );

    std::string dump() const;
    void handle( nlohmann::json data ) const;

private:
    std::size_t callbackId_;
    nlohmann::json message_;
    std::vector< json_handler > handlers_;
};

} // namespace rep
} // namespace prnet

#endif //LIB3DPRNET_REQUEST_HPP
