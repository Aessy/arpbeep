cmake_minimum_required(VERSION 3.1)
project(seqbeep)
add_executable(seqbeep seqbeep.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -ggdb")

add_subdirectory(external/docopt.cpp)

target_link_libraries(seqbeep docopt asound)
