cmake_minimum_required(VERSION 3.16)
project(OpenRGBDarkProjectPlugin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)
find_package(PkgConfig REQUIRED)
pkg_check_modules(HIDAPI REQUIRED hidapi-hidraw)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/OpenRGB/OpenRGBPluginInterface.h")
    set(_DEFAULT_OPENRGB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/OpenRGB")
else()
    set(_DEFAULT_OPENRGB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../OpenRGB")
endif()
set(OPENRGB_DIR "${_DEFAULT_OPENRGB_DIR}" CACHE PATH "Path to OpenRGB 1.1 source tree")

add_library(OpenRGBDarkProjectPlugin SHARED
    DarkProjectPlugin.cpp
    DarkProjectPlugin.h
    RGBController_DarkProjectALU87A.cpp
    RGBController_DarkProjectALU87A.h
    "${OPENRGB_DIR}/RGBController/RGBControllerKeyNames.cpp"
)

target_include_directories(OpenRGBDarkProjectPlugin PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${OPENRGB_DIR}"
    "${OPENRGB_DIR}/RGBController"
    "${OPENRGB_DIR}/dependencies/nlohmann"
    "${OPENRGB_DIR}/dependencies/ghc"
    ${HIDAPI_INCLUDE_DIRS}
)

target_link_libraries(OpenRGBDarkProjectPlugin PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    ${HIDAPI_LIBRARIES}
)

set_target_properties(OpenRGBDarkProjectPlugin PROPERTIES
    PREFIX "lib"
    OUTPUT_NAME "OpenRGBDarkProjectPlugin"
)
