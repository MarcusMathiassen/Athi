cmake_minimum_required(VERSION 3.15)
project(athi)

#set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_STANDARD 17)

#Load headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dep/Universal)

#Load sources
file(GLOB SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Utility/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Physics/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Core/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/dep/Universal/*.cpp
  )

#message("-- CXX ${CMAKE_CXX_COMPILER}")
message("-- Generated with build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CURRENT_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

if(MSVC)
  set(CMAKE_GENERATOR "Visual Studio 15 2017 Win64")

  set(CMAKE_CXX_FLAGS_DEBUG " -DNDEBUG -Ox /MD -W2 /wd4530 /wd4005 /wd4996 /wd4244 /MP")
  set(CMAKE_CXX_FLAGS_RELEASE "-Ox /MD -W3 /wd4530 /wd4005 /wd4996 /wd4244 /MP")

  message("-- flags ${CMAKE_CXX_FLAGS}")
  message("-- Generator set to ${CMAKE_GENERATOR}")
  add_executable(${PROJECT_NAME} ${SOURCES})

  set_target_properties( ${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
  set_target_properties( ${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR} )
  set_target_properties( ${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR} )

  configure_file("./dep/Windows/glew32.dll" ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
  #configure_file("./dep/Windows/glfw3.dll" ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
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
  set_target_properties(glfw PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/dep/Windows/glfw3.lib)

  target_link_libraries(${PROJECT_NAME} freetype opencl glew glfw opengl32.lib user32.lib gdi32.lib kernel32.lib)

elseif(APPLE)

  add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-unused-function )

  #DNDEBUG -fprofile-generate -fprofile-use -fsanitize=address
  set(CMAKE_CXX_FLAGS_DEBUG "-O1 -DNDEBUG -g")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")

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

  # Features required
  target_compile_features(${PROJECT_NAME} PRIVATE cxx_lambda_init_captures)

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

  # Get Boost
  set(Boost_USE_STATIC_LIBS OFF)
  set(Boost_USE_MULTITHREADED ON)
  set(Boost_USE_STATIC_RUNTIME OFF)
  find_package(BOOST 1.66.0 REQUIRED)
  if(BOOST_FOUND)
      include_directories(${BOOST_INCLUDE_DIRS})
      target_link_libraries(${PROJECT_NAME} ${BOOST_LIBRARIES})
  endif()

  # Get Freetype
  find_package(FREETYPE 2.9 REQUIRED)
  if(FREETYPE_FOUND)
      include_directories(${FREETYPE_INCLUDE_DIRS})
      target_link_libraries(${PROJECT_NAME} ${FREETYPE_LIBRARIES})
  endif()

  # Get GLEW
  find_package(GLEW 2.1.0 REQUIRED)
  if(GLEW_FOUND)
      include_directories(${GLEW_INCLUDE_DIRS})
      target_link_libraries(${PROJECT_NAME} ${GLEW_LIBRARIES})
  endif()

  # Get GLFW
  find_package(glfw3 3.3 REQUIRED)
  if (GLFW_FOUND)
      include_directories(${GLFW_INCLUDE_DIR})
      target_link_libraries(${PROJECT_NAME} ${GLFW_LIBRARIES})
  endif()

  # Get OpenGL
  find_package(OPENGL REQUIRED)
  if (OPENGL_FOUND)
      include_directories(${OPENGL_INCLUDE_DIR})
      target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})
  endif()

  # Get OpenCL
  find_package(OPENCL REQUIRED)
  if (OPENCL_FOUND)
      include_directories(${OPENCL_INCLUDE_DIR})
      target_link_libraries(${PROJECT_NAME} ${OPENCL_LIBRARIES})
  endif()

  # LINK TO TARGET
  target_link_libraries(${PROJECT_NAME}
    glfw
    "-framework OpenGL"
    "-framework OpenCL"
    "-framework Cocoa"
    "-framework IOKit"
    "-framework CoreVideo"
    "-framework Foundation"
    )

else()
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

  set(CMAKE_CXX_FLAGS_DEBUG "-march=native -std=c++1z -Wall -Wextra -pedantic -Wno-unused-function -Wno-unused-variable -Wno-unused-parameter")
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
