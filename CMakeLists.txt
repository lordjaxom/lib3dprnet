cmake_minimum_required(VERSION 3.9)
project(lib3dprnet VERSION 0.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-Wa,-mbig-obj")

find_package(Boost 1.66.0 COMPONENTS system coroutine REQUIRED)
set(Boost_DEFINITIONS BOOST_COROUTINES_NO_DEPRECATION_WARNING)

find_path(json_INCLUDE_DIRS nlohmann/json.hpp HINTS "${JSON_ROOT}/include")

add_library(3dprnet SHARED
        core/config.hpp
        core/error.cpp
        core/error.hpp
        core/filesystem.hpp
        core/logging.cpp
        core/logging.hpp
        core/optional.hpp
        core/string_view.hpp
        repetier/service.cpp
        repetier/service.hpp
        repetier/forward.hpp
        repetier/request.cpp
        repetier/request.hpp
        repetier/client.cpp
        repetier/client.hpp
        repetier/types.cpp
        repetier/types.hpp
        repetier/upload.cpp
        repetier/upload.hpp)
target_compile_definitions(3dprnet PUBLIC ${Boost_DEFINITIONS})
target_include_directories(3dprnet PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(3dprnet PUBLIC ${Boost_INCLUDE_DIRS} ${json_INCLUDE_DIRS})
target_link_libraries(3dprnet ${Boost_LIBRARIES})
if (WIN32)
    target_link_libraries(3dprnet stdc++fs ws2_32)
endif()

function(add_test_executable NAME)
    add_executable(${NAME} ${ARGN})
    target_compile_definitions(${NAME} PRIVATE ${Boost_DEFINITIONS})
    target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${Boost_INCLUDE_DIRS})
    target_link_libraries(${NAME} 3dprnet)
    target_link_libraries(${NAME} 3dprnet ${Boost_LIBRARIES})
    if (WIN32)
        target_link_libraries(${NAME} stdc++fs ws2_32)
    endif()
endfunction()

add_test_executable(test_upload test/upload.cpp)
add_test_executable(test_login test/login.cpp)
add_test_executable(test_printers test/printers.cpp)
add_test_executable(test_watch test/watch.cpp)