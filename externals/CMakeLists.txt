cmake_minimum_required(VERSION 3.24...3.31)

include(FetchContent)

macro(set_targets_folder TARGETS FOLDER)
    foreach(target ${TARGETS})
        if(TARGET ${target})
            set_target_properties(${target} PROPERTIES FOLDER ${FOLDER})
        endif()
    endforeach()
endmacro()

if(INPUT_USE_SDL2 AND NOT TARGET SDL2 AND NOT TARGET SDL2::SDL2)
    message(STATUS "Looking for SDL2...")
    FetchContent_Declare(
        SDL2
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-2.32.10
        GIT_SHALLOW TRUE
        FIND_PACKAGE_ARGS QUIET # Use find_package(SDL2 QUIET) to try to find the SDL2 dependency first.
    )
    FetchContent_MakeAvailable(SDL2)
    set_targets_folder(
        "sdl_headers_copy;SDL2;SDL2_test;SDL2main;SDL2-static;uninstall"
        "externals/SDL2"
    )
endif()

if(INPUT_USE_SDL3 AND NOT TARGET SDL3 AND NOT TARGET SDL3::SDL3)
    message(STATUS "Looking for SDL3...")
    FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-3.2.22
        GIT_SHALLOW TRUE
        FIND_PACKAGE_ARGS QUIET # Use find_package(SDL3 QUIET) to try to find the SDL3 dependency first.
    )
    FetchContent_MakeAvailable(SDL3)
    set_targets_folder(
        "SDL_uclibc;SDL3_test;SDL3-shared;SDL3-static"
        "externals/SDL3"
    )
endif()

if(INPUT_USE_GLFW AND NOT TARGET glfw AND NOT TARGET glfw::glfw)
    message(STATUS "Looking for GLFW...")
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
        GIT_SHALLOW TRUE
        FIND_PACKAGE_ARGS NAMES GLFW QUIET # Use find_package(glfw QUIET) to try to find the GLFW dependency first.
    )
    FetchContent_MakeAvailable(glfw)

    if(TARGET glfw AND NOT TARGET glfw::glfw)
        add_library(glfw::glfw ALIAS glfw) # Add alias target if it doesn't exist yet. 
    endif()

    set_targets_folder(
        "glfw;glfw_config;glfw3;glfw3dll;glfw3static;update_mappings"
        "externals/GLFW"
    )
endif()

if(INPUT_BUILD_SAMPLES AND (INPUT_USE_SDL2 OR INPUT_USE_SDL3))
    # We need Freetype for both SDL2_ttf and SDL3_ttf
    message(STATUS "Fetching Freetype...")
    FetchContent_Declare(
        freetype
        GIT_REPOSITORY https://github.com/libsdl-org/freetype.git
        GIT_TAG VER-2-14-1
        GIT_SHALLOW TRUE
        FIND_PACKAGE_ARGS NAMES Freetype FreeType QUIET # Use find_package(freetype QUIET) to try to find the Freetype dependency first.
    )
    FetchContent_MakeAvailable(freetype)

    if(TARGET freetype AND NOT TARGET Freetype::Freetype)
        add_library(Freetype::Freetype ALIAS freetype) # Add alias target if it doesn't exist yet. 
    endif()

    set_targets_folder(
        "freetype;freetype_config;freetype_static"
        "externals/Freetype"
    )

    message(STATUS "Fetching harfbuzz...")
    FetchContent_Declare(
        harfbuzz
        GIT_REPOSITORY https://github.com/libsdl-org/harfbuzz.git
        GIT_TAG 11.5.0
        GIT_SHALLOW TRUE
        FIND_PACKAGE_ARGS QUIET # Use find_package(harfbuzz QUIET) to try to find the Harfbuzz dependency first.
    )
    FetchContent_MakeAvailable(harfbuzz)

    if(TARGET harfbuzz AND NOT TARGET harfbuzz::harfbuzz)
        add_library(harfbuzz::harfbuzz ALIAS harfbuzz) # Add alias target if it doesn't exist yet.
    endif()

    set_targets_folder(
        "harfbuzz;harfbuzz-icu;harfbuzz-subset;harfbuzz-gobject;harfbuzz-cairo"
        "externals/Harfbuzz"
    )

    message(STATUS "Fetching plutosvg...")

    set(PLUTOVG_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # We don't need the examples.
    set(PLUTOSVG_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # We don't need the examples.

    FetchContent_Declare(
        plutosvg
        GIT_REPOSITORY https://github.com/libsdl-org/plutosvg.git
        GIT_TAG v0.0.7
        GIT_SHALLOW TRUE
        FIND_PACKAGE_ARGS QUIET # Use find_package(plutosvg QUIET) to try to find the PlutSVG dependency first.
    )
    FetchContent_MakeAvailable(plutosvg)
    set_targets_folder(
        "plutosvg;plutovg"
        "externals/PlutoSVG"
    )
endif()

if(INPUT_BUILD_SAMPLES AND INPUT_USE_SDL2 AND NOT TARGET SDL2_ttf AND NOT TARGET SDL2_ttf::SDL2_ttf)
    message(STATUS "Fetching SDL2_ttf...")

    set(SDL2TTF_VENDORED OFF CACHE BOOL "" FORCE) # Don't use the provided libraries, as we want to build against our fetched dependencies.

    FetchContent_Declare(
        SDL2_ttf
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
        GIT_TAG release-2.24.0
        GIT_SHALLOW TRUE
        FIND_PACKAGE_ARGS QUIET # Use find_package(SDL2_ttf QUIET) to try to find the SDL2_ttf dependency first.
    )
    FetchContent_MakeAvailable(SDL2_ttf)
    set_targets_folder(
        "SDL2_ttf;SDL2_ttf-shared;SDL2_ttf-static"
        "externals/SDL2_ttf"
    )
endif()

if(INPUT_BUILD_SAMPLES AND INPUT_USE_SDL3 AND NOT TARGET SDL3_ttf AND NOT TARGET SDL3_ttf::SDL3_ttf)
    message(STATUS "Fetching SDL3_ttf...")

    set(SDLTTF_VENDORED OFF CACHE BOOL "" FORCE) # Don't use the provided libraries, as we want to build against our fetched dependencies.

    FetchContent_Declare(
        SDL3_ttf
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
        GIT_TAG release-3.2.2
        GIT_SHALLOW TRUE
        FIND_PACKAGE_ARGS QUIET # Use find_package(SDL3_ttf QUIET) to try to find the SDL3_ttf dependency first.
    )
    FetchContent_MakeAvailable(SDL3_ttf)
    set_targets_folder(
        "SDL3_ttf;SDL3_ttf-shared;SDL3_ttf-static"
        "externals/SDL3_ttf"
    )
endif()
