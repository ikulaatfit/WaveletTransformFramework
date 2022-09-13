cmake_minimum_required(VERSION 3.9.0)

project("FileWriterLib" CXX)

file (GLOB H_FILES src/*.h src/*.hpp)
file (GLOB CPP_FILES src/*.cpp src/*.c src/*.cxx)
add_library(FileWriterLib STATIC ${CPP_FILES} ${H_FILES})

target_include_directories(FileWriterLib PUBLIC src/)

IF(NOT ENABLE_OPENCV)
	option(ENABLE_OPENCV "Enable OpenCV" OFF)
ENDIF()
IF(ENABLE_OPENCV)
  find_package( OpenCV REQUIRED )
  target_compile_definitions(FileWriterLib PUBLIC OPENCV_ENABLED)
  target_include_directories(FileWriterLib PUBLIC ${OpenCV_INCLUDE_DIRS})
  target_link_libraries(FileWriterLib PUBLIC ${OpenCV_LIBS})
ENDIF()

