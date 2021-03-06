#ifndef LIB3DPRNET_REPETIER_UPLOAD_HPP
#define LIB3DPRNET_REPETIER_UPLOAD_HPP

#include <functional>
#include <system_error>

#include <boost/asio/io_context.hpp>

#include "3dprnet/core/config.hpp"
#include "3dprnet/core/filesystem.hpp"
#include "3dprnet/repetier/forward.hpp"

namespace prnet {
namespace rep {

/**
 * function uploadModel
 */

using UploadHandler = std::function< void( std::error_code ec ) >;

void PRNET_DLL uploadModel( boost::asio::io_context &context, Endpoint const &settings, model_ident ident,
                            filesystem::path path, UploadHandler handler = []( auto ec ) {} );

} // namespace rep
} // namespace prnet

#endif // LIB3DPRNET_REPETIER_UPLOAD_HPP
