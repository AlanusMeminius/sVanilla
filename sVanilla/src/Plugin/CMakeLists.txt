cmake_minimum_required(VERSION 3.15)

project(Plugin 
    VERSION 1.0.0 
    LANGUAGES CXX)

find_package(nlohmann_json CONFIG REQUIRED)
set(nlohmann-json_IMPLICIT_CONVERSIONS OFF)

file(GLOB_RECURSE SOURCES "*.cpp" "*.h")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_SOURCE_DIR}/ThirdParty/sVanillaPluginCommon
        ${CMAKE_SOURCE_DIR}/ThirdParty/sVanillaPluginCommon/PluginCommon
)


target_link_libraries(${PROJECT_NAME}
    PRIVATE
        spdlog::spdlog
        nlohmann_json::nlohmann_json
)
