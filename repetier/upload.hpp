#ifndef LIB3DPRNET_REPETIER_UPLOAD_HPP
#define LIB3DPRNET_REPETIER_UPLOAD_HPP

#include <functional>
#include <system_error>

#include <boost/asio/io_context.hpp>

#include "core/config.hpp"
#include "core/filesystem.hpp"
#include "forward.hpp"

namespace prnet {
namespace rep {

/**
 * function upload_model
 */

using upload_callback = std::function< void( std::error_code ) >;

void PRNET_DLL upload_model( boost::asio::io_context& context, settings const& settings, model_ident const& ident,
                             filesystem::path const& path, upload_callback cb );

} // namespace rep
} // namespace prnet

#endif // LIB3DPRNET_REPETIER_UPLOAD_HPP
