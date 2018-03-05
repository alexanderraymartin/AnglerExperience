cmake_minimum_required(VERSION 2.7)

# Name of the project
project(Angler_Experience)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++1y")

# set(CMAKE_BUILD_TYPE DEBUG)

set(GLEW_DIR "$ENV{GLEW_DIR}")
if(NOT GLEW_DIR)
  message(FATAL_ERROR "Please point the environment variable GLEW_DIR to the root directory of your GLEW installation.")
endif()


# Get the GLM environment variable. Since GLM is a header-only library, we
# just need to add it to the include directory.
set(GLM_INCLUDE_DIR "$ENV{GLM_INCLUDE_DIR}")
if(NOT GLM_INCLUDE_DIR)
  MESSAGE(FATAL_ERROR "Please point the environment variable GLM_INCLUDE_DIR to the include directory of your GLM installation.")
endif()


# Get the GLFW environment variable. There should be a CMakeLists.txt in the 
# specified directory.
set(GLFW_DIR "$ENV{GLFW_DIR}")
if(NOT GLFW_DIR)
  message(FATAL_ERROR "Please point the environment variable GLFW_DIR to the root directory of your GLFW3 installation.")
endif()
option(GLFW_BUILD_EXAMPLES "GLFW_BUILD_EXAMPLES" OFF)
option(GLFW_BUILD_TESTS "GLFW_BUILD_TESTS" OFF)
option(GLFW_BUILD_DOCS "GLFW_BUILD_DOCS" OFF)
if(CMAKE_BUILD_TYPE MATCHES Release)
  add_subdirectory(${GLFW_DIR} ${GLFW_DIR}/release)
else()
  add_subdirectory(${GLFW_DIR} ${GLFW_DIR}/debug)
endif()



# Stolen from Ian Dunn
function(BuildProperties TargetName)
  if(WIN32)
    # With prebuilt binaries
    link_directories(${GLEW_DIR}/lib/Release/Win32)
  endif()

  target_include_directories(${TargetName} PUBLIC ${GLM_INCLUDE_DIR})

  target_include_directories(${TargetName} PUBLIC ${GLFW_DIR}/include)

  target_link_libraries(${TargetName} glfw ${GLFW_LIBRARIES})

  # Get the GLEW environment variable.
  target_include_directories(${TargetName} PUBLIC  ${GLEW_DIR}/include)
  if(WIN32)
    target_link_libraries(${TargetName} ${GLEW_DIR}/lib/Release/Win32/glew32s.lib)
  else()
    target_link_libraries(${TargetName} ${GLEW_DIR}/lib/libGLEW.a)
  endif()

  # OS specific options and libraries
  if(WIN32)
    # c++0x is enabled by default.
    # -Wall produces way too many warnings.
    # -pedantic is not supported.
    target_link_libraries(${TargetName} opengl32.lib)
  else()
    # Enable all pedantic warnings.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -pedantic")
    if(APPLE)
      # Add required frameworks for GLFW.
      target_link_libraries(${TargetName} "-framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo")
    else()
      #Link the Linux OpenGL library
      target_link_libraries(${TargetName} "GL")
    endif()
  endif()

  # Include header only Json library
  target_include_directories(${TargetName} PUBLIC "${PROJECT_SOURCE_DIR}/libs/JSON/")

  target_include_directories(${TargetName} PUBLIC "${PROJECT_SOURCE_DIR}/src/utility/")

endfunction(BuildProperties)



# HERE begins the actual main CMake setup

add_subdirectory("src/utility")

add_subdirectory("libs/TinyObj")

file(GLOB SOURCES "src/*.cpp" "src/components/*.cpp")

# We don't really need to include header and resource files to build, but it's
# nice to have them show up in IDEs.
file(GLOB_RECURSE HEADERS "src/*.h" "src/*.hpp")
file(GLOB_RECURSE GLSL "shaders/*.vs" "shaders/*.fs" "shaders/*.gs" "shaders/*.cs")

add_executable(${CMAKE_PROJECT_NAME} ${SOURCES} ${HEADERS} ${GLSL})

include_directories("${PROJECT_SOURCE_DIR}/src/components")

# Sets the APPTYPE constant so that the executable will compile. See core.h for context
target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC APPTYPE=ApplicationType::GAME)
add_definitions("-DASSET_DIR=${CMAKE_SOURCE_DIR}/gameassets" "-DSHADER_DIR=${CMAKE_SOURCE_DIR}/shaders")

BuildProperties(${CMAKE_PROJECT_NAME})

target_link_libraries(${CMAKE_PROJECT_NAME} utilityTarget)

target_link_libraries(${CMAKE_PROJECT_NAME} tinyObjTarget)

include_directories("${PROJECT_SOURCE_DIR}/libs/TinyObj")

#Custom build command to put all shaders into a single JSON file.
set(SHADERPACKAGE "shaders/shaders.json") 
add_custom_command(OUTPUT ${SHADERPACKAGE} COMMAND python jsify_shaders.py shaders/ -o ${SHADERPACKAGE} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DEPENDS ${GLSL})
add_custom_target(Shader_Pack DEPENDS ${SHADERPACKAGE})
add_dependencies(${CMAKE_PROJECT_NAME} Shader_Pack)
