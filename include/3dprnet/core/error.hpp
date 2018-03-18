#ifndef LIB3DPRNET_CORE_ERROR_HPP
#define LIB3DPRNET_CORE_ERROR_HPP

#include <system_error>
#include <type_traits>

#include "3dprnet/core/config.hpp"

namespace prnet {

/**
 * enum class prnet_errc
 */

enum class prnet_errc : int
{
    not_ok,
    exception,
    server_error,
    protocol_violation,
    timeout
};


/**
 * function prnet_category
 */

namespace detail {

class PRNET_DLL prnet_category
        : public std::error_category
{
public:
    char const* name() const noexcept override { return "prnet::prnet_category"; }

    std::string message( int value ) const override;
};

} // namespace detail

std::error_category const& PRNET_DLL prnet_category();


/**
 * function make_error_code
 */

std::error_code PRNET_DLL make_error_code( prnet_errc e );

} // namespace prnet


/**
 * namespace std hooks
 */

namespace std {

template<>
struct is_error_code_enum< prnet::prnet_errc >
        : public std::true_type {};

} // namespace std

#endif // LIB3DPRNET_CORE_ERROR_HPP
