
string(TIMESTAMP BEFORE "%s")

CPMAddPackage(
    NAME "glm"
    URL "https://github.com/g-truc/glm/archive/refs/tags/1.0.1.zip"
)

find_package(glm REQUIRED)
set (glm_INCLUDE_DIR ${glm_SOURCE_DIR}/glm)
include_directories(${glm_INCLUDE_DIR})

string(TIMESTAMP AFTER "%s")
math(EXPR DELTAglm "${AFTER} - ${BEFORE}")
MESSAGE(STATUS "glm TIME: ${DELTAglm}s")