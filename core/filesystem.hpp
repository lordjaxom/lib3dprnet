#ifndef LIB3DPRNET_CORE_FILESYSTEM_HPP
#define LIB3DPRNET_CORE_FILESYSTEM_HPP

#if __cplusplus >= 201700L
#   include <filesystem>
#   define PRNET_FILESYSTEM_NAMESPACE std::filesystem
#elif defined(__has_include) && __has_include(<experimental/filesystem>)
#   include <experimental/filesystem>
#   define PRNET_FILESYSTEM_NAMESPACE std::experimental::filesystem
#else
#   include <boost/filesystem.hpp>
#   define PRNET_FILESYSTEM_NAMESPACE boost::filesystem
#endif

namespace prnet {

namespace filesystem = PRNET_FILESYSTEM_NAMESPACE;
#undef PRNET_FILESYSTEM_NAMESPACE

} // namespace prnet

#endif // LIB3DPRNET_CORE_FILESYSTEM_HPP
