#ifndef LIB3DPRNET_CORE_FILESYSTEM_HPP
#define LIB3DPRNET_CORE_FILESYSTEM_HPP

#include <string>

#include "3dprnet/core/config.hpp"

#if __cplusplus >= 201703L
#   include <filesystem>
#   define PRNET_FILESYSTEM_NAMESPACE std::filesystem
#elif __has_include( <experimental/filesystem> )
#   include <experimental/filesystem>
#   define PRNET_FILESYSTEM_NAMESPACE std::experimental::filesystem
#else
#   include <boost/filesystem.hpp>
#   define PRNET_FILESYSTEM_NAMESPACE boost::filesystem
#endif

namespace prnet {
namespace filesystem {

using namespace PRNET_FILESYSTEM_NAMESPACE;

std::string PRNET_DLL native_path( path const& path );

} // namespace filesystem
} // namespace prnet

#undef PRNET_FILESYSTEM_NAMESPACE

#endif // LIB3DPRNET_CORE_FILESYSTEM_HPP
