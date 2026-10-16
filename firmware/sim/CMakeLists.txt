cmake_minimum_required(VERSION 3.16)
project(bayupandu_sim C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

get_filename_component(FIRMWARE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.. ABSOLUTE)

include(FetchContent)
FetchContent_Declare(lvgl
    GIT_REPOSITORY https://github.com/lvgl/lvgl.git
    GIT_TAG v8.3.11
    GIT_SHALLOW TRUE
    SOURCE_SUBDIR none)  # fetch only; LVGL is built below with the firmware's lv_conf.h
FetchContent_MakeAvailable(lvgl)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)

# LVGL, built against the firmware's own lv_conf.h.
file(GLOB_RECURSE LVGL_SOURCES ${lvgl_SOURCE_DIR}/src/*.c)
add_library(lvgl STATIC ${LVGL_SOURCES})
target_include_directories(lvgl PUBLIC ${lvgl_SOURCE_DIR} ${lvgl_SOURCE_DIR}/..)
target_compile_definitions(lvgl PUBLIC
    LV_CONF_PATH=${FIRMWARE_DIR}/lv_conf.h  # LVGL stringifies this itself
    LV_LVGL_H_INCLUDE_SIMPLE)

# The firmware sources the device build actually uses. Screens.cpp is not
# instantiated by the firmware, and main.cpp is replaced by sim_main.cpp.
file(GLOB FIRMWARE_SERVICES ${FIRMWARE_DIR}/src/Services/*.cpp)
set(FIRMWARE_UI
    ${FIRMWARE_DIR}/src/UI/BootChime.cpp
    ${FIRMWARE_DIR}/src/UI/BootScreen.cpp
    ${FIRMWARE_DIR}/src/UI/BrandMark.cpp
    ${FIRMWARE_DIR}/src/UI/InputManager.cpp
    ${FIRMWARE_DIR}/src/UI/LVGLDisplayDriver.cpp
    ${FIRMWARE_DIR}/src/UI/LVGLInit.cpp
    ${FIRMWARE_DIR}/src/UI/LVGLInputDriver.cpp
    ${FIRMWARE_DIR}/src/UI/SignGlyph.cpp
    ${FIRMWARE_DIR}/src/UI/UserInterface.cpp)
file(GLOB FIRMWARE_FONTS ${FIRMWARE_DIR}/src/UI/fonts/*.c)

add_executable(bayupandu_sim sim_main.cpp ${FIRMWARE_SERVICES} ${FIRMWARE_UI} ${FIRMWARE_FONTS})
target_include_directories(bayupandu_sim PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/shim
    ${FIRMWARE_DIR}/src)
target_link_libraries(bayupandu_sim PRIVATE lvgl PkgConfig::SDL2)
target_compile_definitions(bayupandu_sim PRIVATE
    BAYUPANDU_EXAMPLE_IGC="${FIRMWARE_DIR}/../documentation/example/complex_example_lxn.igc")
