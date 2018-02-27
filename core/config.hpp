#ifndef LIB3DPRNET_CORE_CONFIG_HPP
#define LIB3DPRNET_CORE_CONFIG_HPP

#if defined( _3dprnet_EXPORTS )
#   define PRNET_DLL __declspec( dllexport )
#else
#   define PRNET_DLL __declspec( dllimport )
#endif

#endif //LIB3DPRNET_CORE_CONFIG_HPP
