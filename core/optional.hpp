#ifndef LIB3DPRNET_CORE_OPTIONAL_HPP
#define LIB3DPRNET_CORE_OPTIONAL_HPP

#if __cplusplus >= 201700L
#   include <optional>
#   define PRNET_OPTIONAL_NAMESPACE std
#elif defined(__has_include) && __has_include(<experimental/optional>)
#   include <experimental/optional>
#   define PRNET_OPTIONAL_NAMESPACE std::experimental
#else
#   include <boost/utility/string_view.hpp>
#   define PRNET_OPTIONAL_NAMESPACE boost
#endif

namespace prnet {

template< typename T >
using optional = PRNET_OPTIONAL_NAMESPACE::optional< T >;

} // namespace prnet

#undef PRNET_OPTIONAL_NAMESPACE

#endif // LIB3DPRNET_CORE_OPTIONAL_HPP
