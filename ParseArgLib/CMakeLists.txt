cmake_minimum_required(VERSION 3.9.0)

project("ParseArgLib" CXX)

file (GLOB H_FILES src/*.h src/*.hpp)
file (GLOB CPP_FILES src/*.cpp src/*.c src/*.cxx)
add_library(ParseArgLib STATIC ${CPP_FILES} ${H_FILES})

target_include_directories(ParseArgLib PUBLIC src/)

