cmake_minimum_required(VERSION 3.2)
project(athi)

set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_STANDARD 17)

#Load headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dep/Universal)

#Load sources
file(GLOB SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Utility/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Physics/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Core/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/dep/Universal/*.cpp
  )

message("-- CXX ${CMAKE_CXX_COMPILER}")
message("-- Generated with build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CURRENT_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

#Copy the resources to our binary directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Resources DESTINATION  ${CMAKE_CURRENT_BINARY_DIR})


if(WIN32)

  set(CMAKE_GENERATOR "Visual Studio 15 2017 Win64")

  set(CMAKE_CXX_FLAGS_DEBUG "-Ox /MD -DNDEBUG -W1 /wd4530 /wd4005 /wd4996 /wd4244 /MP")
  set(CMAKE_CXX_FLAGS_RELEASE "-Ox /MD -W3 /wd4530 /wd4005 /wd4996 /wd4244 /MP")

  message("-- flags ${CMAKE_CXX_FLAGS}")
  message("-- Generator set to ${CMAKE_GENERATOR}")
  add_executable(${PROJECT_NAME} ${SOURCES})

  set_target_properties( ${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
  set_target_properties( ${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR} )
  set_target_properties( ${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR} )

  configure_file("./dep/Windows/glew32.dll" ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
  configure_file("./dep/Windows/glfw3.dll" ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
  configure_file("./dep/Windows/freetype271.dll" ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
  configure_file("./dep/Windows/OpenCL.dll" ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

  set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

  # INCLUDE DEPENDENCIES
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dep/Windows)

  add_library(opencl STATIC IMPORTED)
  set_target_properties(opencl PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/dep/Windows/OpenCL.lib)

  add_library(glew STATIC IMPORTED)
  set_target_properties(glew PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/dep/Windows/glew32.lib)

  add_library(freetype STATIC IMPORTED)
  set_target_properties(freetype PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/dep/Windows/freetype271.lib)

  add_library(glfw STATIC IMPORTED)
  set_target_properties(glfw PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/dep/Windows/glfw3dll.lib)

  target_link_libraries(${PROJECT_NAME} freetype opencl glew glfw opengl32.lib user32.lib gdi32.lib kernel32.lib)

elseif(APPLE)

  set(CMAKE_CXX_FLAGS_DEBUG "-DNDEBUG -Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-variable -Wno-unused-parameter")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter")

  message("-- Debug   flags: ${CMAKE_CXX_FLAGS_DEBUG}")
  message("-- Release flags: ${CMAKE_CXX_FLAGS_RELEASE}")
  message("-- Generator set to: ${CMAKE_GENERATOR}")

  # RESOURCE FILE LOCATION
  file(GLOB RESOURCE_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Fonts/*.ttf"
  "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Fonts/*.ttc"
  "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Kernels/*.cl"
  "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Textures/*.jpg"
  "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Textures/*.png"
  "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shaders/*.vert"
  "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shaders/*.frag"
  "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shaders/*.geom"
  "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shaders/*.comp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Shaders/*.glsl"
  )

  add_executable(${PROJECT_NAME} ${SOURCES} ${RESOURCE_FILES})

  # BUNDLE OPTIONS
  set_target_properties(${PROJECT_NAME} PROPERTIES
  MACOSX_BUNDLE false
  MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Resources/info.plist
  #MACOSX_BUNDLE_ICON_FILE ${CMAKE_CURRENT_SOURCE_DIR}/Resources/icon_16.png
  MACOSX_FRAMEWORK_IDENTIFIER org.cmake.${PROJECT_NAME}
  RESOURCE "${RESOURCE_FILES}"
  )

  # INCLUDE DEPENDENCIES
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dep/macOS)

  add_library(glew STATIC IMPORTED)
  set_target_properties(glew PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/dep/macOS/GL/libGLEW.a)

  add_library(glfw STATIC IMPORTED)
  set_target_properties(glfw PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/dep/macOS/GLFW/libglfw3.a)

  # LINK TO TARGET
  target_link_libraries(${PROJECT_NAME} glew glfw "-lpthread -lfreetype -framework OpenGL -framework OpenCL -framework Cocoa -framework IOKIT -framework CoreVideo")

else()
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

  set(CMAKE_CXX_FLAGS_DEBUG "-O3 -march=native -std=c++1z -Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-variable -Wno-unused-parameter")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -std=c++1z -Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-parameter")

  # RESOURCE FILE LOCATION
  file(GLOB RESOURCE_FILES
  "./Resources/*.png"
  "./Resources/*.vert"
  "./Resources/*.frag"
  "./Resources/*.ttf"
  "./Resources/*.cl"
  )

  add_executable(${PROJECT_NAME} ${SOURCES} ${RESOURCE_FILES})
  # INCLUDE DEPENDENCIES
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dep/universal)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dep/Unix)

  add_library(glew STATIC IMPORTED)
  set_target_properties(glew PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/dep/Unix/GL/libGLEW.a)

  add_library(glfw STATIC IMPORTED)
  set_target_properties(glfw PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/dep/Unix/GLFW/libglfw3.a)

  # LINK TO TARGET
  target_link_libraries(${PROJECT_NAME} glew glfw "-lpthread -lOpenCL -lGL -lGLU -lX11 -ldl")

endif()
