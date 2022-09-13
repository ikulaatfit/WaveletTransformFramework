cmake_minimum_required(VERSION 3.9.0)

project("OpenclHelperLib" CXX) 

file (GLOB H_FILES src/*.h src/*.hpp)
file (GLOB CPP_FILES src/*.cpp src/*.c src/*.cxx)
add_library(OpenclHelperLib STATIC ${CPP_FILES} ${H_FILES})

target_include_directories(OpenclHelperLib PUBLIC src/)
set_property(TARGET OpenclHelperLib PROPERTY CXX_STANDARD 17)

option(ENABLE_COMPUTE_CACHE "Compute Cache" ON)
IF(ENABLE_COMPUTE_CACHE)
 target_compile_definitions(OpenclHelperLib PUBLIC COMPUTE_CACHE_ENABLED)
ENDIF()
