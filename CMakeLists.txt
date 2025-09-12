cmake_minimum_required( VERSION 3.12...3.31 )

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option( INPUT_BUILD_SAMPLES "Build Input samples." ON )

set( INPUT_VERSION_MAJOR 0 )
set( INPUT_VERSION_MINOR 0 )
set( INPUT_VERSION_PATCH 1 )
set( INPUT_VERSION ${INPUT_VERSION_MAJOR}.${INPUT_VERSION_MINOR}.${INPUT_VERSION_PATCH})

# Use solution folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project( input VERSION ${INPUT_VERSION} LANGUAGES C CXX )

# Enable multi-processor builds in MSVC.
if(MSVC)
    add_compile_options(/MP)
endif(MSVC)

set( INC_FILES
    inc/input/Gamepad.hpp
    inc/input/Input.hpp
    inc/input/Keyboard.hpp
    inc/input/Mouse.hpp
)

set( SRC_FILES
    src/Gamepad.cpp
    src/Keyboard.cpp
    src/Mouse.cpp
)

find_package(GDK)

if(GDK_FOUND)
    message(STATUS "GDK found, enabling GDK support.")
    set( GDK_SRC_FILES
        src/GDK/GamepadGDK.cpp
        src/GDK/InputGDK.cpp
        src/GDK/KeyboardGDK.cpp
        src/GDK/MouseGDK.cpp
    )

    source_group(GDK FILES ${GDK_SRC_FILES})

    list(APPEND SRC_FILES ${GDK_SRC_FILES})
else()
    message(STATUS "GDK not found, GDK support disabled.")
endif()

set( ALL_FILES 
    ${INC_FILES} 
    ${SRC_FILES} 
    .clang-format
)

add_library( input STATIC ${ALL_FILES} )
add_library( input::input ALIAS input )

target_compile_features( input PUBLIC cxx_std_20 )

if(GDK_FOUND)
    target_link_libraries( input 
        PUBLIC Microsoft::GDK
    )
endif()

target_include_directories( input 
    PUBLIC inc
)

