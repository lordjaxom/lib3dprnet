cmake_minimum_required(VERSION 3.9)
project(asynctest)

include(CMakeLocal.cmake)

set(CMAKE_CXX_STANDARD 14)

find_package(Boost 1.66.0 COMPONENTS system coroutine REQUIRED)
set(Boost_DEFINITIONS BOOST_COROUTINES_NO_DEPRECATION_WARNING)

add_executable(asynctest main.cpp)
target_compile_definitions(asynctest PRIVATE ${Boost_DEFINITIONS})
target_include_directories(asynctest PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(asynctest ${Boost_LIBRARIES})
if (WIN32)
    target_compile_definitions(asynctest PRIVATE WIN32_LEAN_AND_MEAN)
    target_link_libraries(asynctest ws2_32)
endif()