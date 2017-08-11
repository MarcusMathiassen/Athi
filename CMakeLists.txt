cmake_minimum_required(VERSION 3.9.0)
project(athi)

# Compiler flags

set(CMAKE_CXX_STANDARD 17)

SET(CMAKE_BUILD_TYPE DEBUG)
#SET(CMAKE_BUILD_TYPE RELEASE)

set(DEBUG TRUE)

if(${DEBUG})
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)
else()
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
endif()

# Load headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/)

# Load sources
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

if(WIN32)
	add_executable(${PROJECT_NAME} ${SOURCES})

  # INCLUDE DEPENDENCIES
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dep/Windows)

  ADD_LIBRARY(glew STATIC IMPORTED)
  SET_TARGET_PROPERTIES(glew PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/dep/Windows/GL/libGLEW.a)

  ADD_LIBRARY(glfw STATIC IMPORTED)
  SET_TARGET_PROPERTIES(glfw PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/dep/Windows/GLFW/libglfw3.a)
	target_link_libraries(${PROJECT_NAME} "-lopengl32 -lkernel32")

elseif(APPLE)

  SET(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-variable -Wno-unused-parameter")
  SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter")

	# RESOURCE FILE LOCATION
	file(GLOB RESOURCE_FILES
  "./Resources/*.png"
  "./Resources/*.vs"
  "./Resources/*.fs"
	)

	add_executable(${PROJECT_NAME} ${SOURCES} ${RESOURCE_FILES})

	# BUNDLE OPTIONS
	set_target_properties(${PROJECT_NAME} PROPERTIES
  MACOSX_BUNDLE FALSE
  MACOSX_FRAMEWORK_IDENTIFIER org.cmake.${PROJECT_NAME}
  RESOURCE "${RESOURCE_FILES}"
	)

	# INCLUDE DEPENDENCIES
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dep/macOS)

	ADD_LIBRARY(glew STATIC IMPORTED)
	SET_TARGET_PROPERTIES(glew PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/dep/macOS/GL/libGLEW.a)

	ADD_LIBRARY(glfw STATIC IMPORTED)
	SET_TARGET_PROPERTIES(glfw PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/dep/macOS/GLFW/libglfw3.a)

	# LINK TO TARGET
	target_link_libraries(${PROJECT_NAME} glew glfw "-framework OpenGL -framework OpenCL -framework Cocoa -framework IOKIT -framework CoreVideo")

endif()
