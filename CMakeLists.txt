cmake_minimum_required(VERSION 3.12...3.31)

option(INPUT_BUILD_SAMPLES "Build Input samples." ON)

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

if(WIN32)
    message(STATUS "Win32 found, Win32 support enabled.")
    set( WIN32_SRC_FILES
        src/backends/Win32/GamepadWin32.cpp
        src/backends/Win32/KeyboardWin32.cpp
        src/backends/Win32/MouseWin32.cpp
    )
    source_group(backends/Win32 FILES ${WIN32_SRC_FILES})

    add_library( input_Win32 STATIC ${INC_FILES} ${SRC_FILES} ${WIN32_SRC_FILES} .clang-format)
    add_library( input::Win32 ALIAS input_Win32)
    target_compile_features(input_Win32 PUBLIC cxx_std_20)

    target_link_libraries( input_Win32
    )

    target_include_directories( input_Win32
        PUBLIC inc
    )
else()
    message(STATUS "Win32 not found, Win32 support disabled.")
endif()

find_package(GDK)

if(GDK_FOUND)
    message(STATUS "GDK found, GDK support enabled.")
    set(GDK_SRC_FILES
        src/backends/GDK/GamepadGDK.cpp
        src/backends/GDK/KeyboardGDK.cpp
        src/backends/GDK/MouseGDK.cpp
    )

    source_group(backends/GDK FILES ${GDK_SRC_FILES})

    add_library( input_GDK STATIC ${INC_FILES} ${SRC_FILES} ${GDK_SRC_FILES} .clang-format)
    add_library( input::GDK ALIAS input_GDK)
    target_compile_features(input_GDK PUBLIC cxx_std_20)
    target_link_libraries( input_GDK
        PUBLIC Microsoft::GDK
    )

    target_include_directories( input_GDK
        PUBLIC inc
    )

else()
    message(STATUS "GDK not found, GDK support disabled.")
endif()

find_package(SDL2)

if(SDL2_FOUND)
    message(STATUS "SDL2 found, SDL2 support enabled.")
        set(SDL2_SRC_FILES
        src/backends/SDL2/GamepadSDL2.cpp
        src/backends/SDL2/KeyboardSDL2.cpp
        src/backends/SDL2/MouseSDL2.cpp
    )

    source_group(backends/SDL2 FILES ${SDL2_SRC_FILES})

    add_library( input_SDL2 STATIC ${INC_FILES} ${SRC_FILES} ${SDL2_SRC_FILES} .clang-format)
    add_library( input::SDL2 ALIAS input_SDL2)
    target_compile_features(input_SDL2 PUBLIC cxx_std_20)
    target_link_libraries( input_SDL2
        PUBLIC SDL2::SDL2
    )

    target_include_directories( input_SDL2
        PUBLIC inc
    )

else()
    message(STATUS "SDL2 not found, SDL2 support disabled.")
endif()

find_package(SDL3)

if(SDL3_FOUND)
    message(STATUS "SDL3 found, SDL3 support enabled.")
    set(SDL3_SRC_FILES
        src/backends/SDL3/GamepadSDL3.cpp
        src/backends/SDL3/KeyboardSDL3.cpp
        src/backends/SDL3/MouseSDL3.cpp
    )

    source_group(backends/SDL3 FILES ${SDL3_SRC_FILES})

    add_library( input_SDL3 STATIC ${INC_FILES} ${SRC_FILES} ${SDL3_SRC_FILES} .clang-format)
    add_library( input::SDL3 ALIAS input_SDL3)
    target_compile_features(input_SDL3 PUBLIC cxx_std_20)
    target_link_libraries( input_SDL3
        PUBLIC SDL3::SDL3
    )

    target_include_directories( input_SDL3
        PUBLIC inc
    )

else()
    message(STATUS "SDL3 not found, SDL3 support disabled.")
endif()

find_package(GLFW)

if(GLFW_FOUND)
    message(STATUS "GLFW found, GLFW support enabled.")
	set( GLFW_SRC_FILES
        src/backends/GLFW/GamepadGLFW.cpp
        src/backends/GLFW/KeyboardGLFW.cpp
        src/backends/GLFW/MouseGLFW.cpp
    )

    source_group(backends/GLFW FILES ${GLFW_SRC_FILES})

    add_library( input_GLFW STATIC ${INC_FILES} ${SRC_FILES} ${GLFW_SRC_FILES} .clang-format)
    add_library( input::GLFW ALIAS input_GLFW)
    target_compile_features(input_GLFW PUBLIC cxx_std_20)
    target_link_libraries( input_GLFW
        PUBLIC glfw
    )

    target_include_directories( input_GLFW
        PUBLIC inc
	)
else()
    message(STATUS "GLFW not found, GLFW support disabled.")
endif()

if(INPUT_BUILD_SAMPLES)
    add_subdirectory(samples)
endif()