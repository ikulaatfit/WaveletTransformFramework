cmake_minimum_required(VERSION 3.9.0)

project("WaveletTransformLib" CXX)

file (GLOB H_FILES src/*.h src/*.hpp)
file (GLOB CPP_FILES src/*.cpp src/*.c src/*.cxx)
add_library(WaveletTransformLib STATIC ${CPP_FILES} ${H_FILES})

target_include_directories(WaveletTransformLib PUBLIC src/)

IF(NOT ENABLE_OPENCL)
	option(ENABLE_OPENCL "Enable OpenCL" OFF)
ENDIF()
IF(ENABLE_OPENCL)
  find_package( OpenCL REQUIRED )
  target_compile_definitions(WaveletTransformLib PUBLIC OPENCL_ENABLED)
  target_include_directories(WaveletTransformLib PUBLIC ${OpenCL_INCLUDE_DIRS})
  target_link_libraries(WaveletTransformLib PUBLIC ${OpenCL_LIBRARIES})
ENDIF()

add_subdirectory(OpenclHelperLib)
target_link_libraries(WaveletTransformLib PUBLIC OpenclHelperLib) 