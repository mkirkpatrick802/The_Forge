
string(TIMESTAMP BEFORE "%s")

CPMAddPackage(
    NAME nlohmann_json
    VERSION 3.9.1
    URL https://github.com/nlohmann/json/releases/download/v3.9.1/include.zip
    DOWNLOAD_ONLY YES
)

add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE ${nlohmann_json_SOURCE_DIR}/include)

string(TIMESTAMP AFTER "%s")
math(EXPR DELTAjson "${AFTER} - ${BEFORE}")
MESSAGE(STATUS "json TIME: ${DELTAjson}s")