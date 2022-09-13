cmake_minimum_required(VERSION 3.9.0)

project("WindowRendererLib" CXX) 

file (GLOB H_FILES src/*.h src/*.hpp)
file (GLOB CPP_FILES src/*.cpp src/*.c src/*.cxx)
add_library(WindowRendererLib STATIC ${CPP_FILES} ${H_FILES})

target_include_directories(WindowRendererLib PUBLIC src/)

IF(NOT ENABLE_OPENCV)
	option(ENABLE_OPENCV "Enable OpenCV" OFF)
ENDIF()
IF(ENABLE_OPENCV)
  find_package( OpenCV REQUIRED )
  target_compile_definitions(WindowRendererLib PUBLIC OPENCV_ENABLED)
  target_include_directories(WindowRendererLib PUBLIC ${OpenCV_INCLUDE_DIRS})
  target_link_libraries(WindowRendererLib PUBLIC ${OpenCV_LIBS})
ENDIF()

IF(NOT ENABLE_SDL)
	option(ENABLE_SDL "Enable SDL" OFF)
ENDIF()
IF(ENABLE_SDL)
  find_package(SDL2 REQUIRED)
  find_package(SDL2_ttf REQUIRED)
  target_compile_definitions(WindowRendererLib PUBLIC SDL_ENABLED)
  target_include_directories(WindowRendererLib PUBLIC ${SDL2_INCLUDE_DIR} ${SDL2_TTF_INCLUDE_DIR})
  target_link_libraries(WindowRendererLib PUBLIC ${SDL2_LIBRARY} ${SDL2_TTF_LIBRARIES})
ENDIF()

