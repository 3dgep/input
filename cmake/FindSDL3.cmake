# FindSDL3.cmake
find_path(SDL3_INCLUDE_DIR 
    NAMES SDL.h SDL3/SDL.h
    HINTS
        ENV SDL3DIR
        ${SDL3_ROOT}
    PATH_SUFFIXES 
        SDL3
        include/SDL3
        include
)

find_library(SDL3_LIBRARY
    NAMES SDL3 SDL3-static SDL3-3.0
    HINTS
        ENV SDL3DIR
        ${SDL3_ROOT}
    PATH_SUFFIXES
        lib
        lib64
        lib/x64
        lib/x86
)

# SDL3 doesn't have a separate SDL3main library like SDL2
set(SDL3_LIBRARIES ${SDL3_LIBRARY})
set(SDL3_INCLUDE_DIRS ${SDL3_INCLUDE_DIR})

# Try to get version
if(SDL3_INCLUDE_DIR AND EXISTS "${SDL3_INCLUDE_DIR}/SDL_version.h")
    file(READ "${SDL3_INCLUDE_DIR}/SDL_version.h" SDL3_VERSION_CONTENT)
    string(REGEX MATCH "#define SDL_MAJOR_VERSION[ \t]+([0-9]+)" _ ${SDL3_VERSION_CONTENT})
    set(SDL3_VERSION_MAJOR ${CMAKE_MATCH_1})
    string(REGEX MATCH "#define SDL_MINOR_VERSION[ \t]+([0-9]+)" _ ${SDL3_VERSION_CONTENT})
    set(SDL3_VERSION_MINOR ${CMAKE_MATCH_1})
    string(REGEX MATCH "#define SDL_PATCHLEVEL[ \t]+([0-9]+)" _ ${SDL3_VERSION_CONTENT})
    set(SDL3_VERSION_PATCH ${CMAKE_MATCH_1})
    set(SDL3_VERSION "${SDL3_VERSION_MAJOR}.${SDL3_VERSION_MINOR}.${SDL3_VERSION_PATCH}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL3
    REQUIRED_VARS SDL3_LIBRARY SDL3_INCLUDE_DIR
    VERSION_VAR SDL3_VERSION
)

if(SDL3_FOUND AND NOT TARGET SDL3::SDL3)
    add_library(SDL3::SDL3 UNKNOWN IMPORTED)
    set_target_properties(SDL3::SDL3 PROPERTIES
        IMPORTED_LOCATION "${SDL3_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${SDL3_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(SDL3_INCLUDE_DIR SDL3_LIBRARY)