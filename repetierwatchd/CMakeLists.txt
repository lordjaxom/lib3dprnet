cmake_minimum_required(VERSION 3.6)
project(repetierwatchd)

include(CMakeLocal.cmake)

set(CMAKE_CXX_STANDARD 14)

find_package(Boost COMPONENTS "system"  REQUIRED)

find_path(beast_INCLUDE_DIRS boost/beast.hpp HINTS "${BEAST_ROOT}/include")
find_path(json_INCLUDE_DIRS nlohmann/json.hpp HINTS "${JSON_ROOT}/include")

set(SOURCE_FILES
        main.cpp
        repetier_socket.cpp
        repetier_socket.hpp
        repetier_action.cpp
        repetier_action.hpp)

add_executable(repetierwatchd ${SOURCE_FILES})
target_include_directories(repetierwatchd PRIVATE ${Boost_INCLUDE_DIRS} ${beast_INCLUDE_DIRS} ${json_INCLUDE_DIRS})
target_link_libraries(repetierwatchd ${Boost_LIBRARIES})
if(WIN32)
    target_link_libraries(repetierwatchd ws2_32 stdc++fs)
endif()