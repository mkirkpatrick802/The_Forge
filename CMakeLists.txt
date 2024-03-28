cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

# policies
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# create project
project(The_Forge)

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
include(external/glm.cmake)
include(external/json.cmake)

execute_process(COMMAND cmd /C "${CMAKE_SOURCE_DIR}/vcpkg_setup.bat")

find_package(Protobuf CONFIG REQUIRED)
find_package(GameNetworkingSockets CONFIG REQUIRED)

# add executable
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/public/*.cpp")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "src/private/*.hpp")

add_executable(Engine ${SOURCES})

target_include_directories(Engine PRIVATE "src/private" "src/libs")

find_package(OpenGL REQUIRED)

# link SDL2, OpenGL, ImGUI, GLM, and JSON
target_link_libraries(Engine 
    PUBLIC SDL2-static ${OPENGL_gl_LIBRARY} ${CMAKE_DL_LIBS} SDL2_image IMGUI glew glm nlohmann_json
)

# link GameNetworkingSockets
target_link_libraries(Engine
    PRIVATE GameNetworkingSockets::static GameNetworkingSockets::GameNetworkingSockets
    PRIVATE protobuf::libprotobuf
)

add_custom_command(TARGET Engine PRE_BUILD

        # Remove the existing directory first
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets"

        # Then copy the new directory contents over
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets"

        COMMENT "Replacing asset data in the build folder"
)

add_custom_command(TARGET Engine PRE_BUILD

        COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_SOURCE_DIR}/out"
        COMMENT "Deleting extra out folder"
)