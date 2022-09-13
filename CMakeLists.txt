cmake_minimum_required(VERSION 3.9.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake/Modules/") 

project("WaveletTransform" CXX)

option(ENABLE_FILE_WRITER "Enable File Writer" OFF)
option(ENABLE_WINDOW_RENDERER "Enable Window Displayer" OFF)


find_package( OpenCL REQUIRED )
set(ENABLE_OPENCL ON)
include_directories(${OpenCL_INCLUDE_DIRS})

file (GLOB CPP_FILES src/*.cpp src/*.h)
add_executable(WaveletTransform ${CPP_FILES})

IF(ENABLE_FILE_WRITER OR ENABLE_WINDOW_RENDERER)
	find_package( OpenCV REQUIRED )
	include_directories(${OpenCV_INCLUDE_DIRS})
	target_link_libraries(WaveletTransform ${OpenCV_LIBS})
	set(ENABLE_OPENCV ON)
ENDIF()

add_subdirectory(FileReaderLib)
IF(ENABLE_FILE_WRITER)
	add_subdirectory(FileWriterLib)
	target_link_libraries(WaveletTransform FileWriterLib) 
	target_compile_definitions(WaveletTransform PUBLIC FILE_WRITER_ENABLED)
ENDIF()
IF(ENABLE_WINDOW_RENDERER)
	add_subdirectory(WindowRendererLib)
	target_link_libraries(WaveletTransform WindowRendererLib) 
	target_compile_definitions(WaveletTransform PUBLIC WINDOW_RENDERER_ENABLED)
ENDIF()


#set(THREADS_PREFER_PTHREAD_FLAG ON)
#find_package(Threads REQUIRED)
find_package(OpenMP)
if(OPENMP_FOUND)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()


add_subdirectory(ParseArgLib)
add_subdirectory(WaveletTransformLib)

set_property(TARGET WaveletTransform PROPERTY CXX_STANDARD 14)
target_link_libraries(WaveletTransform ${OpenCL_LIBRARIES} FileReaderLib ParseArgLib WaveletTransformLib)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
add_custom_command(TARGET WaveletTransform POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/WindowRendererLib/assets $<TARGET_FILE_DIR:WaveletTransform>)
add_custom_command(TARGET WaveletTransform POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/WaveletTransformLib/assets $<TARGET_FILE_DIR:WaveletTransform>)
