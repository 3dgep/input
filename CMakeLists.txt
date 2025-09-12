cmake_minimum_required(VERSION 3.12...3.31)

option(INPUT_BUILD_SAMPLES "Build Input samples." ON)
set(INPUT_BACKEND "" CACHE STRING "Which backend to use.")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(INPUT_VERSION_MAJOR 0)
set(INPUT_VERSION_MINOR 0)
set(INPUT_VERSION_PATCH 1)
set(INPUT_VERSION ${INPUT_VERSION_MAJOR}.${INPUT_VERSION_MINOR}.${INPUT_VERSION_PATCH})

# Use solution folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(input VERSION ${INPUT_VERSION} LANGUAGES C CXX)

# Enable multi-processor builds in MSVC.
if(MSVC)
    add_compile_options(/MP)
endif(MSVC)

set(INC_FILES
    inc/input/Gamepad.hpp
    inc/input/Input.hpp
    inc/input/Keyboard.hpp
    inc/input/Mouse.hpp
)

set(SRC_FILES
    src/Gamepad.cpp
    src/Keyboard.cpp
    src/Mouse.cpp
)

set(AVAILABLE_BACKENDS "")

if(WIN32)
    list(APPEND AVAILABLE_BACKENDS "WIN32")
endif()

find_package(GDK)

if(GDK_FOUND)
    message(STATUS "GDK found, GDK support enabled.")
    list(APPEND AVAILABLE_BACKENDS "GDK")
else()
    message(STATUS "GDK not found, GDK support disabled.")
endif()

find_package(SDL2)

if(SDL2_FOUND)
    message(STATUS "SDL2 found, SDL2 support enabled.")
    list(APPEND AVAILABLE_BACKENDS "SDL2")
else()
    message(STATUS "SDL2 not found, SDL2 support disabled.")
endif()

find_package(SDL3)

if(SDL3_FOUND)
    message(STATUS "SDL3 found, SDL3 support enabled.")
    list(APPEND AVAILABLE_BACKENDS "SDL3")
else()
    message(STATUS "SDL3 not found, SDL3 support disabled.")
endif()

find_package(GLFW)

if(GLFW_FOUND)
    message(STATUS "GLFW found, GLFW support enabled.")
    list(APPEND AVAILABLE_BACKENDS "GLFW")
else()
    message(STATUS "GLFW not found, GLFW support disabled.")
endif()

set_property(CACHE INPUT_BACKEND PROPERTY STRINGS ${AVAILABLE_BACKENDS})

if(INPUT_BACKEND STREQUAL "GDK")
    set(GDK_SRC_FILES
        src/backends/GDK/GamepadGDK.cpp
        src/backends/GDK/InputGDK.cpp
        src/backends/GDK/KeyboardGDK.cpp
        src/backends/GDK/MouseGDK.cpp
    )
    list(APPEND SRC_FILES ${GDK_SRC_FILES})

    source_group(backends/GDK FILES ${GDK_SRC_FILES})
endif()

set(ALL_FILES
    ${INC_FILES}
    ${SRC_FILES}
    .clang-format
)

add_library(input STATIC ${ALL_FILES})
add_library(input::input ALIAS input)

target_compile_features(input PUBLIC cxx_std_20)

if(GDK_FOUND)
    target_link_libraries(input
        PUBLIC Microsoft::GDK
    )
endif()

target_include_directories(input
    PUBLIC inc
)

