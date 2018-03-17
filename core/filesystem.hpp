#ifndef LIB3DPRNET_CORE_FILESYSTEM_HPP
#define LIB3DPRNET_CORE_FILESYSTEM_HPP

#if __has_include( <filesystem> )
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

namespace filesystem = PRNET_FILESYSTEM_NAMESPACE;

} // namespace prnet

#undef PRNET_FILESYSTEM_NAMESPACE

#endif // LIB3DPRNET_CORE_FILESYSTEM_HPP
