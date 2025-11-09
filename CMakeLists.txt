cmake_minimum_required(VERSION 3.12...3.31)

option(INPUT_BUILD_SAMPLES "Build Input samples." OFF)
option(INPUT_USE_SDL2 "Use SDL2 backend." OFF)
option(INPUT_USE_SDL3 "Use SDL3 backend." OFF)
option(INPUT_USE_GDK "Use GDK backend." OFF)
option(INPUT_USE_GLFW "Use GLFW backend." OFF)
option(INPUT_USE_WIN32 "Use Win32 backend." OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(INPUT_VERSION_MAJOR 1)
set(INPUT_VERSION_MINOR 0)
set(INPUT_VERSION_PATCH 0)
set(INPUT_VERSION ${INPUT_VERSION_MAJOR}.${INPUT_VERSION_MINOR}.${INPUT_VERSION_PATCH})

# Use solution folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(input VERSION ${INPUT_VERSION} LANGUAGES C CXX)

# Enable multi-processor builds in MSVC.
if(MSVC)
    add_compile_options(/MP)
endif(MSVC)

add_subdirectory(externals EXCLUDE_FROM_ALL)

set(INC_FILES
    inc/input/ButtonState.hpp
    inc/input/Gamepad.hpp
    inc/input/Input.hpp
    inc/input/Keyboard.hpp
    inc/input/Mouse.hpp
    inc/input/Touch.hpp
)

set(SRC_FILES
    src/Gamepad.cpp
    src/Input.cpp
    src/Keyboard.cpp
    src/Mouse.cpp
    src/Touch.cpp
)

if(INPUT_USE_WIN32 AND WIN32)
    message(STATUS "Win32 found, Win32 support enabled.")
    set(WIN32_SRC_FILES
        src/backends/Win32/GamepadWin32.cpp
        src/backends/Win32/KeyboardWin32.cpp
        src/backends/Win32/MouseWin32.cpp
        src/backends/Win32/TouchWin32.cpp
    )
    source_group(backends/Win32 FILES ${WIN32_SRC_FILES})

    add_library(input_Win32 STATIC ${INC_FILES} ${SRC_FILES} ${WIN32_SRC_FILES} .clang-format)
    add_library(input::Win32 ALIAS input_Win32)
    target_compile_features(input_Win32 PUBLIC cxx_std_20)

    target_include_directories(input_Win32
        PUBLIC inc
    )
else()
    message(STATUS "Win32 support disabled.")
endif()

if(INPUT_USE_GDK)
    find_package(GDK)

    if(GDK_FOUND)
        message(STATUS "GDK found, GDK support enabled.")
        set(GDK_SRC_FILES
            src/backends/GDK/GamepadGDK.cpp
            src/backends/GDK/KeyboardGDK.cpp
            src/backends/GDK/MouseGDK.cpp
            src/backends/GDK/TouchGDK.cpp
        )

        source_group(backends/GDK FILES ${GDK_SRC_FILES})

        add_library(input_GDK STATIC ${INC_FILES} ${SRC_FILES} ${GDK_SRC_FILES} .clang-format)
        add_library(input::GDK ALIAS input_GDK)
        target_compile_features(input_GDK PUBLIC cxx_std_20)
        target_link_libraries(input_GDK
            PUBLIC Microsoft::GDK
        )

        target_include_directories(input_GDK
            PUBLIC inc
        )

    else()
        message(STATUS "GDK not found, GDK support disabled.")
    endif()
else()
    message(STATUS "GDK support disabled.")
endif()

if(INPUT_USE_SDL2)
    if(TARGET SDL2::SDL2)
        set(SDL2_SRC_FILES
            src/backends/SDL2/GamepadSDL2.cpp
            src/backends/SDL2/KeyboardSDL2.cpp
            src/backends/SDL2/MouseSDL2.cpp
            src/backends/SDL2/TouchSDL2.cpp
        )

        source_group(backends/SDL2 FILES ${SDL2_SRC_FILES})

        add_library(input_SDL2 STATIC ${INC_FILES} ${SRC_FILES} ${SDL2_SRC_FILES} .clang-format)
        add_library(input::SDL2 ALIAS input_SDL2)
        target_compile_features(input_SDL2 PUBLIC cxx_std_20)
        target_link_libraries(input_SDL2
            PUBLIC SDL2::SDL2
        )

        target_include_directories(input_SDL2
            PUBLIC inc
        )
    else()
        message(STATUS "Could not find SDL2, support disabled.")
    endif()
endif()

if(INPUT_USE_SDL3)
    if(TARGET SDL3::SDL3)
        set(SDL3_SRC_FILES
            src/backends/SDL3/GamepadSDL3.cpp
            src/backends/SDL3/KeyboardSDL3.cpp
            src/backends/SDL3/MouseSDL3.cpp
            src/backends/SDL3/TouchSDL3.cpp
        )

        source_group(backends/SDL3 FILES ${SDL3_SRC_FILES})

        add_library(input_SDL3 STATIC ${INC_FILES} ${SRC_FILES} ${SDL3_SRC_FILES} .clang-format)
        add_library(input::SDL3 ALIAS input_SDL3)
        target_compile_features(input_SDL3 PUBLIC cxx_std_20)
        target_link_libraries(input_SDL3
            PUBLIC SDL3::SDL3
        )

        target_include_directories(input_SDL3
            PUBLIC inc
        )
    else()
        message(STATUS "SDL3 not found, support disabled.")
    endif()
endif()

if(INPUT_USE_GLFW)
    if(TARGET glfw::glfw)
        set(GLFW_SRC_FILES
            src/backends/GLFW/GamepadGLFW.cpp
            src/backends/GLFW/KeyboardGLFW.cpp
            src/backends/GLFW/MouseGLFW.cpp
            src/backends/GLFW/TouchGLFW.cpp
        )

        source_group(backends/GLFW FILES ${GLFW_SRC_FILES})

        add_library(input_GLFW STATIC ${INC_FILES} ${SRC_FILES} ${GLFW_SRC_FILES} .clang-format)
        add_library(input::GLFW ALIAS input_GLFW)
        target_compile_features(input_GLFW PUBLIC cxx_std_20)
        target_link_libraries(input_GLFW
            PUBLIC glfw::glfw
        )

        target_include_directories(input_GLFW
            PUBLIC inc
        )
    else()
        message(STATUS "Could not find GLFW, support disabled.")
    endif()
endif()

if(INPUT_BUILD_SAMPLES)
    add_subdirectory(samples)
endif()
