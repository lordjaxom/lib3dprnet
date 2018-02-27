#ifndef LIB3DPRNET_REQUEST_HPP
#define LIB3DPRNET_REQUEST_HPP

#include <functional>
#include <memory>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

#include "core/config.hpp"
#include "forward.hpp"

namespace prnet {
namespace rep {

namespace detail {

template< typename T >
auto request_callback(
        T&& callback,
        std::enable_if_t< std::is_constructible< std::function< void () >, T >::value >* = nullptr )
{
    return [callback { std::forward( callback ) } ]( auto const& ) { callback(); };
}

template< typename T >
auto request_callback( T&& callback )
{
    return [callback { std::forward< T >( callback ) } ]( auto const& data ) { callback( data ); };
}

} // namespace detail

/**
 * class request
 */

class PRNET_DLL request
{
    struct internal_t {};

    static constexpr internal_t internal {};

public:
    using handler_type = std::function< void ( nlohmann::json const& ) >;

    static handler_type check_ok_flag();

    request( std::string action, callback<> callback );
    ~request();

    template< typename Callback >
    request( std::string action, Callback callback )
            : request( internal, std::move( action ), detail::request_callback( std::move( callback ) ) ) {}

    void printer( std::string printer );
    void callback_id( size_t id );

    template< typename T >
    void set( std::string key, T&& value )
    {
        message_[ "data" ].emplace( std::move( key ), std::forward< T >( value ) );
    }

    void add_handler( handler_type handler );

    std::string dump() const;
    void handle( nlohmann::json const& data ) const;

private:
    request( internal_t, std::string&& action, callback< nlohmann::json const& >&& callback );

    nlohmann::json message_;
    std::vector< handler_type > handlers_;
    callback< nlohmann::json const& > callback_;
};

} // namespace rep
} // namespace prnet

#endif //LIB3DPRNET_REQUEST_HPP
