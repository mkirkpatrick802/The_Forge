cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# policies
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# create project
project(MyProject)

# set where the compiled files goes
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libs)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libs)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
LINK_DIRECTORIES(${CMAKE_BINARY_DIR}/libs)

# add dependencies
include(external/compilerchecks.cmake)
include(external/cpm.cmake)

# add libraries
include(external/glew.cmake)
include(external/sdl.cmake)
include(external/imgui.cmake)

# add executable
add_executable(demo main.cpp)

find_package(OpenGL REQUIRED)

target_link_libraries(demo PUBLIC SDL2-static ${OPENGL_gl_LIBRARY} ${CMAKE_DL_LIBS} SDL2_image IMGUI glew)