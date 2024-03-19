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

find_package(protobuf CONFIG REQUIRED)
find_package(GameNetworkingSockets CONFIG REQUIRED)

# add executable
add_executable(demo main.cpp)

find_package(OpenGL REQUIRED)

# link SDL2, OpenGL, and ImGUI
target_link_libraries(demo 
    PUBLIC SDL2-static ${OPENGL_gl_LIBRARY} ${CMAKE_DL_LIBS} SDL2_image IMGUI glew
)

# link GameNetworkingSockets
target_link_libraries(demo
    PRIVATE GameNetworkingSockets::static GameNetworkingSockets::GameNetworkingSockets
    PRIVATE protobuf::libprotobuf
)