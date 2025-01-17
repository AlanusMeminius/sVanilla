cmake_minimum_required(VERSION 3.15)

project(SUI 
    VERSION 1.0.0 
    LANGUAGES CXX)

set(QT_COMPONENTS
    Core
    Gui
    Widgets
    Svg
)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS ${QT_COMPONENTS})


file(GLOB_RECURSE SOURCES "*.cpp" "*.h")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_SOURCE_DIR}/ThirdParty/sVanillaPluginCommon/PluginCommon
)

foreach (COMPONENTS ${QT_COMPONENTS})
    set(QT_LIBS ${QT_LIBS} Qt${QT_VERSION_MAJOR}::${COMPONENTS})
endforeach ()

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${QT_LIBS}
        spdlog::spdlog
        BaseQt
)
