cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# policies
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

execute_process(COMMAND vcpkg_setup.bat)

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

# set(OPENSSL_ROOT_DIR vcpkg/packages/openssl_x64-windows)

# set(protobuf_DIR vcpkg/packages/protobuf_x64-windows/share/protobuf)
# set(Protobuf_INCLUDE_DIR vcpkg/packages/protobuf_x64-windows/include)

# set(GameNetworkingSockets_DIR vcpkg/packages/gamenetworkingsockets_x64-windows/share/gamenetworkingsockets)

find_package(protobuf CONFIG REQUIRED)
find_package(GameNetworkingSockets CONFIG REQUIRED)

# add executable
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/public/*.cpp")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "src/private/*.hpp")

add_executable(Engine ${SOURCES})

target_include_directories(Engine PRIVATE "src/private")

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