#ifndef LIB3DPRNET_REPETIER_UPLOAD_HPP
#define LIB3DPRNET_REPETIER_UPLOAD_HPP

#include <functional>

#include <boost/asio/io_context.hpp>

#include "core/config.hpp"
#include "core/filesystem.hpp"
#include "forward.hpp"

namespace prnet {
namespace rep {

/**
 * function upload_model
 */

void PRNET_DLL upload_model( boost::asio::io_context& context, settings const& settings, model_ident const& ident,
                   filesystem::path const& path, callback< std::error_code > callback );

} // namespace rep
} // namespace prnet

#endif // LIB3DPRNET_REPETIER_UPLOAD_HPP
