#ifndef LIB3DPRNET_REPETIER_UPLOAD_HPP
#define LIB3DPRNET_REPETIER_UPLOAD_HPP

#include <functional>

#include <boost/asio/io_context.hpp>

#include "core/filesystem.hpp"
#include "types.hpp"

namespace prnet {
namespace rep {

/**
 * function upload_model
 */

void upload_model( boost::asio::io_context& context, settings const& settings, model_ident const& ident,
                   prnet::filesystem::path const& path, callback<> callback );

} // namespace rep
} // namespace prnet

#endif // LIB3DPRNET_REPETIER_UPLOAD_HPP
