# FindGDK.cmake
# Finds the Microsoft Game Development Kit

# Check various environment variables in order of preference
if(DEFINED ENV{GRDKLatest})
    set(GDK_ROOT_PATH "$ENV{GRDKLatest}")
    set(GDK_TYPE "GRDK")
elseif(DEFINED ENV{GameDKLatest})
    set(GDK_ROOT_PATH "$ENV{GameDKLatest}")
    set(GDK_TYPE "GameDK")
elseif(DEFINED ENV{GXDKLatest})
    set(GDK_ROOT_PATH "$ENV{GXDKLatest}")
    set(GDK_TYPE "GXDK")
endif()

# Allow user to override with GDK_ROOT
if(GDK_ROOT)
    set(GDK_ROOT_PATH "${GDK_ROOT}")
endif()

if(GDK_ROOT_PATH)
    message(STATUS "Searching for GDK in: ${GDK_ROOT_PATH}")
    
    # Find GDK headers - check multiple possible locations
    find_path(GDK_INCLUDE_DIR
        NAMES XGameRuntime.h
        PATHS
            ${GDK_ROOT_PATH}/GameKit/Include
            ${GDK_ROOT_PATH}/GRDK/GameKit/Include
            ${GDK_ROOT_PATH}/GXDK/GameKit/Include
        NO_DEFAULT_PATH
    )
    
    # Determine architecture
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(GDK_ARCH "amd64")
    else()
        set(GDK_ARCH "x86")
    endif()
    
    # Find GDK libraries - check multiple possible locations
    find_library(GDK_LIBRARY
        NAMES xgameruntime.lib XGameRuntime.lib
        PATHS
            ${GDK_ROOT_PATH}/GameKit/Lib/${GDK_ARCH}
            ${GDK_ROOT_PATH}/GRDK/GameKit/Lib/${GDK_ARCH}
            ${GDK_ROOT_PATH}/GXDK/GameKit/Lib/${GDK_ARCH}
        NO_DEFAULT_PATH
    )
        # Find GameInput library
        find_library(GAMEINPUT_LIBRARY
            NAMES GameInput.lib
            PATHS
                ${GDK_ROOT_PATH}/GameKit/Lib/${GDK_ARCH}
                ${GDK_ROOT_PATH}/GRDK/GameKit/Lib/${GDK_ARCH}
                ${GDK_ROOT_PATH}/GXDK/GameKit/Lib/${GDK_ARCH}
            NO_DEFAULT_PATH
        )
    
    # Try to read version file if it exists
    if(EXISTS "${GDK_ROOT_PATH}/GDKVersion.txt")
        file(READ "${GDK_ROOT_PATH}/GDKVersion.txt" GDK_VERSION_STRING)
        string(STRIP "${GDK_VERSION_STRING}" GDK_VERSION)
    elseif(EXISTS "${GDK_ROOT_PATH}/version.txt")
        file(READ "${GDK_ROOT_PATH}/version.txt" GDK_VERSION_STRING)
        string(STRIP "${GDK_VERSION_STRING}" GDK_VERSION)
    endif()
endif()

# Handle the QUIETLY and REQUIRED arguments
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GDK
    REQUIRED_VARS GDK_INCLUDE_DIR GDK_LIBRARY GDK_ROOT_PATH
    VERSION_VAR GDK_VERSION
)

if(GDK_FOUND)
    set(GDK_INCLUDE_DIRS ${GDK_INCLUDE_DIR})
    if(GAMEINPUT_LIBRARY)
        set(GDK_LIBRARIES ${GDK_LIBRARY} ${GAMEINPUT_LIBRARY})
    else()
        set(GDK_LIBRARIES ${GDK_LIBRARY})
    endif()
    
    message(STATUS "GDK Type: ${GDK_TYPE}")
    message(STATUS "GDK Include: ${GDK_INCLUDE_DIR}")
    message(STATUS "GDK Libraries: ${GDK_LIBRARIES}")
    
    # Create imported target
    if(NOT TARGET Microsoft::GDK)
        add_library(Microsoft::GDK UNKNOWN IMPORTED)
        set_target_properties(Microsoft::GDK PROPERTIES
            IMPORTED_LOCATION "${GDK_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${GDK_INCLUDE_DIR}"
        )
        if(GAMEINPUT_LIBRARY)
            set_property(TARGET Microsoft::GDK APPEND PROPERTY IMPORTED_LINK_INTERFACE_LIBRARIES "${GAMEINPUT_LIBRARY}")
        endif()
    endif()
endif()

mark_as_advanced(GDK_INCLUDE_DIR GDK_LIBRARY)