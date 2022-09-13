cmake_minimum_required(VERSION 3.9.0)

project("FileReaderLib" CXX)

file (GLOB H_FILES src/*.h src/*.hpp)
file (GLOB CPP_FILES src/*.cpp src/*.c src/*.cxx)
add_library(FileReaderLib STATIC ${CPP_FILES} ${H_FILES})

target_include_directories(FileReaderLib PUBLIC src/)
#target_include_directories(${PROJECT_NAME} PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
#target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)

IF(NOT ENABLE_OPENCV)
	option(ENABLE_OPENCV "Enable OpenCV" OFF)
ENDIF()
IF(ENABLE_OPENCV)
  find_package( OpenCV REQUIRED )
  target_compile_definitions(FileReaderLib PUBLIC OPENCV_ENABLED)
  target_include_directories(FileReaderLib PUBLIC ${OpenCV_INCLUDE_DIRS})
  target_link_libraries(FileReaderLib PUBLIC ${OpenCV_LIBS})
ENDIF()

IF(NOT ENABLE_FFMPEG)
	option(ENABLE_FFMPEG "Enable FFMpeg" OFF)
ENDIF()
IF(ENABLE_FFMPEG)
  find_package( FFmpeg REQUIRED )
  target_compile_definitions(FileReaderLib PUBLIC FFMPEG_ENABLED)
  target_include_directories(FileReaderLib PUBLIC ${FFMPEG_INCLUDE_DIRS})
  target_link_libraries(FileReaderLib PUBLIC ${FFMPEG_LIBRARIES})
ENDIF()

