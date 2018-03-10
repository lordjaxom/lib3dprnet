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
 * class Request
 */

class PRNET_DLL Request
{
public:
    using ResponseHandler = std::function< void ( nlohmann::json const& data ) >;
    using CleanupHandler = std::function< void () >;

    Request( std::string action, ResponseHandler handler );
    Request( Request const& ) = delete;
    Request( Request&& other );
    ~Request();

    std::size_t callbackId() const { return callbackId_; }

    void printer( std::string slug );

    template< typename T >
    void set( std::string key, T&& value )
    {
        message_[ "data" ].emplace( std::move( key ), std::forward< T >( value ) );
    }

    void cleanup_handler( CleanupHandler handler );

	std::string build( std::size_t callbackId );
    void handle( nlohmann::json const& data ) const;

private:
    nlohmann::json message_;
    ResponseHandler responseHandler_;
    CleanupHandler cleanupHandler_;
    std::size_t callbackId_ {};
};

} // namespace rep
} // namespace prnet

#endif //LIB3DPRNET_REQUEST_HPP
