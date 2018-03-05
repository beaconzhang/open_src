project(SMHasher)

cmake_minimum_required(VERSION 2.4)

set(CMAKE_BUILD_TYPE Release)

#add_library(
#  SMHasherSupport
#  Types.cpp
#  PMurHash.c
#)

add_executable(
   hashtable
   test.cpp 
)

#target_link_libraries(
#  SMHasher
#  SMHasherSupport
#)
