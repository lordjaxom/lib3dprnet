cmake_minimum_required(VERSION 3.9)
project(lib3dprnet VERSION 0.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-Wa,-mbig-obj")

find_package(Boost 1.66.0 COMPONENTS system coroutine REQUIRED)
set(Boost_DEFINITIONS BOOST_COROUTINES_NO_DEPRECATION_WARNING)

find_path(json_INCLUDE_DIRS nlohmann/json.hpp HINTS "${JSON_ROOT}/include")

add_library(3dprnet STATIC
        core/error.cpp
        core/error.hpp
        core/filesystem.hpp
        core/optional.hpp
        core/string_view.hpp
        repetier/types.cpp
        repetier/types.hpp
        repetier/socket.cpp
        repetier/socket.hpp
        repetier/upload.cpp
        repetier/upload.hpp repetier/request.cpp repetier/request.hpp)
target_compile_definitions(3dprnet PUBLIC ${Boost_DEFINITIONS})
target_include_directories(3dprnet PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(3dprnet PUBLIC ${Boost_INCLUDE_DIRS} ${json_INCLUDE_DIRS})

add_executable(test_upload test/upload.cpp)
target_compile_definitions(test_upload PRIVATE ${Boost_DEFINITIONS})
target_include_directories(test_upload PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${Boost_INCLUDE_DIRS})
target_link_libraries(test_upload 3dprnet)
target_link_libraries(test_upload 3dprnet ${Boost_LIBRARIES})
if (WIN32)
    target_link_libraries(3dprnet stdc++fs ws2_32)
endif()

add_executable(test_login test/login.cpp)
target_compile_definitions(test_login PRIVATE ${Boost_DEFINITIONS})
target_include_directories(test_login PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${Boost_INCLUDE_DIRS})
target_link_libraries(test_login 3dprnet)
target_link_libraries(test_login 3dprnet ${Boost_LIBRARIES})
if (WIN32)
    target_link_libraries(3dprnet stdc++fs ws2_32)
endif()
