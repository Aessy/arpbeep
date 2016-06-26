cmake_minimum_required(VERSION 3.1)
project(seqbeep)
add_executable(seqbeep src/seqbeep.cpp src/MidiDriver.cpp src/Sequencer.cpp src/log.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -ggdb")

target_link_libraries(seqbeep boost_log boost_system boost_log_setup boost_thread asound nana pthread X11 Xft)
