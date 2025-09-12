# FindSDL2.cmake
# Locate SDL2 library

find_path(SDL2_INCLUDE_DIR SDL.h
    HINTS
        ENV SDL2DIR
        ${SDL2_ROOT}
    PATH_SUFFIXES 
        SDL2
        include/SDL2
        include
)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(VC_LIB_PATH_SUFFIX lib/x64)
else()
    set(VC_LIB_PATH_SUFFIX lib/x86)
endif()

find_library(SDL2_LIBRARY
    NAMES SDL2 SDL2-static
    HINTS
        ENV SDL2DIR
        ${SDL2_ROOT}
    PATH_SUFFIXES
        lib
        ${VC_LIB_PATH_SUFFIX}
)

find_library(SDL2_MAIN_LIBRARY
    NAMES SDL2main
    HINTS
        ENV SDL2DIR
        ${SDL2_ROOT}
    PATH_SUFFIXES
        lib
        ${VC_LIB_PATH_SUFFIX}
)

set(SDL2_LIBRARIES ${SDL2_LIBRARY} ${SDL2_MAIN_LIBRARY})
set(SDL2_INCLUDE_DIRS ${SDL2_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2
    REQUIRED_VARS SDL2_LIBRARY SDL2_INCLUDE_DIR
)

if(SDL2_FOUND AND NOT TARGET SDL2::SDL2)
    add_library(SDL2::SDL2 UNKNOWN IMPORTED)
    set_target_properties(SDL2::SDL2 PROPERTIES
        IMPORTED_LOCATION "${SDL2_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIR}"
    )
    
    if(SDL2_MAIN_LIBRARY)
        add_library(SDL2::SDL2main UNKNOWN IMPORTED)
        set_target_properties(SDL2::SDL2main PROPERTIES
            IMPORTED_LOCATION "${SDL2_MAIN_LIBRARY}"
        )
    endif()
endif()

mark_as_advanced(SDL2_INCLUDE_DIR SDL2_LIBRARY SDL2_MAIN_LIBRARY)