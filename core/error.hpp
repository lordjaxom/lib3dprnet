#ifndef LIB3DPRNET_CORE_ERROR_HPP
#define LIB3DPRNET_CORE_ERROR_HPP

#include <system_error>
#include <type_traits>

namespace prnet {

/**
 * enum class errc
 */

enum class errc : int
{
    server_error
};


/**
 * function prnet_category
 */

namespace detail {

class prnet_category
        : public std::error_category
{
public:
    char const* name() const noexcept override { return "prnet::prnet_category"; }

    std::string message( int value ) const override;
};

} // namespace detail

std::error_category const& prnet_category();


/**
 * function make_error_code
 */

std::error_code make_error_code( errc e );

} // namespace prnet


/**
 * namespace std hooks
 */

namespace std {

template<>
struct is_error_code_enum< prnet::errc >
        : public std::true_type {};

} // namespace std

#endif // LIB3DPRNET_CORE_ERROR_HPP
