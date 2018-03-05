cmake_minimum_required(VERSION 2.8)

project(QUERY_SERVER_TOOL)

set(CMAKE_C_CFLAGS "-Wall ")
set(CMAKE_CXX_FLAGS "-O3 -std=c++0x -w -fpermissive -Wl,--export-dynamic -Wno-deprecated")
set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH}":/opt/meituan/")

add_definitions(-DHAVE_NETINET_IN_H)
add_definitions(-DHAVE_NETDB_H)
add_definitions(-DWRITE_TO_FILE)

include_directories(
    #    src/MTSearch_thrift/gen-cpp
    #/usr/include/thrift
)

cmake_policy(SET CMP0015 NEW)

add_subdirectory(diff)
