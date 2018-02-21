cmake_minimum_required(VERSION 3.9)
project(lib3dprnet VERSION 0.1)

set(CMAKE_CXX_STANDARD 14)

find_package(Boost 1.66.0 COMPONENTS system coroutine REQUIRED)
set(Boost_DEFINITIONS BOOST_COROUTINES_NO_DEPRECATION_WARNING)

add_library(3dprnet SHARED
        core/error.cpp
        core/error.hpp
        core/filesystem.hpp
        repetier/types.cpp
        repetier/types.hpp
        repetier/upload.cpp
        repetier/upload.hpp)
target_compile_definitions(3dprnet PUBLIC ${Boost_DEFINITIONS})
target_include_directories(3dprnet PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(3dprnet PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(3dprnet ${Boost_LIBRARIES})
if (WIN32)
    target_link_libraries(3dprnet ws2_32)
endif()
