cmake_minimum_required(VERSION 3.12...3.31)

option(INPUT_BUILD_SAMPLES "Build Input samples." OFF)
option(INPUT_USE_SDL2 "Use SDL2 backend." OFF)
option(INPUT_USE_SDL3 "Use SDL3 backend." OFF)
option(INPUT_USE_GDK "Use GDK backend." OFF)
option(INPUT_USE_GLFW "Use GLFW backend." OFF)
option(INPUT_USE_WIN32 "Use Win32 backend." OFF)

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

if(INPUT_USE_WIN32 AND WIN32)
    message(STATUS "Win32 found, Win32 support enabled.")
    set(WIN32_SRC_FILES
        src/backends/Win32/GamepadWin32.cpp
        src/backends/Win32/KeyboardWin32.cpp
        src/backends/Win32/MouseWin32.cpp
    )
    source_group(backends/Win32 FILES ${WIN32_SRC_FILES})

    add_library(input_Win32 STATIC ${INC_FILES} ${SRC_FILES} ${WIN32_SRC_FILES} .clang-format)
    add_library(input::Win32 ALIAS input_Win32)
    target_compile_features(input_Win32 PUBLIC cxx_std_20)

    target_link_libraries(input_Win32
    )

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

macro( set_targets_folder TARGETS FOLDER )
	foreach( target ${TARGETS} )
		if( TARGET ${target} )
			set_target_properties( ${target} PROPERTIES FOLDER ${FOLDER})
		endif()
	endforeach()
endmacro()


if(INPUT_USE_SDL2)
    find_package(SDL2)

    if(NOT SDL2_FOUND)
        include(FetchContent)
        FetchContent_Declare(
            SDL2
            GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
            GIT_TAG release-2.32.10
        )
        FetchContent_MakeAvailable(SDL2)
        set_targets_folder(
            "sdl_headers_copy;SDL2;SDL2_test;SDL2main;SDL2-static;uninstall"
            "externals/SDL2"
        )
    endif()

    set(SDL2_SRC_FILES
        src/backends/SDL2/GamepadSDL2.cpp
        src/backends/SDL2/KeyboardSDL2.cpp
        src/backends/SDL2/MouseSDL2.cpp
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
    message(STATUS "SDL2 support disabled.")
endif()

if(INPUT_USE_SDL3)
    find_package(SDL3)

    if(NOT SDL3_FOUND)
        include(FetchContent)
        FetchContent_Declare(
            SDL3
            GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
            GIT_TAG release-3.2.22
        )
        FetchContent_MakeAvailable(SDL3)
        set_targets_folder(
            "SDL_uclibc;SDL3_test;SDL3-shared;SDL3-static"
            "externals/SDL3"
        )
    endif()

    set(SDL3_SRC_FILES
        src/backends/SDL3/GamepadSDL3.cpp
        src/backends/SDL3/KeyboardSDL3.cpp
        src/backends/SDL3/MouseSDL3.cpp
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
    message(STATUS "SDL3 support disabled.")
endif()


if(INPUT_USE_GLFW)
    find_package(GLFW)

    if(NOT GLFW_FOUND)
        include(FetchContent)
        FetchContent_Declare(
            GLFW
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG 3.4
        )
        FetchContent_MakeAvailable(GLFW)
        set_targets_folder(
            "glfw;update_mappings"
            "externals/GLFW"
        )
    endif()

    set(GLFW_SRC_FILES
        src/backends/GLFW/GamepadGLFW.cpp
        src/backends/GLFW/KeyboardGLFW.cpp
        src/backends/GLFW/MouseGLFW.cpp
    )

    source_group(backends/GLFW FILES ${GLFW_SRC_FILES})

    add_library(input_GLFW STATIC ${INC_FILES} ${SRC_FILES} ${GLFW_SRC_FILES} .clang-format)
    add_library(input::GLFW ALIAS input_GLFW)
    target_compile_features(input_GLFW PUBLIC cxx_std_20)
    target_link_libraries(input_GLFW
        PUBLIC glfw
    )

    target_include_directories(input_GLFW
        PUBLIC inc
    )
else()
    message(STATUS "GLFW support disabled.")
endif()

if(INPUT_BUILD_SAMPLES)
    add_subdirectory(samples)
endif()
