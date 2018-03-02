#ifndef LIB3DPRNET_REPETIER_FORWARD_HPP
#define LIB3DPRNET_REPETIER_FORWARD_HPP

#include <functional>
#include <system_error>

#include "core/config.hpp"

namespace prnet {
namespace rep {

class model_ident;
class printer;
class request;
class settings;
class client;

/**
 * callbacks
 */

template< typename ...Args >
using callback = std::function< void ( Args... ) >;

using success_callback = callback<>;
using connect_callback = callback< std::error_code >;
using error_callback = callback< std::error_code >;

} // namespace rep
} // namespace prnet

#endif // LIB3DPRNET_REPETIER_FORWARD_HPP
