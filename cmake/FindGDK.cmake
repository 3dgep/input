# FindGDK.cmake
# Finds the Microsoft Game Development Kit

# Check environment variable first
if(DEFINED ENV{GameDKLatest})
    set(GDK_ROOT_PATH "$ENV{GameDKLatest}")
endif()

# Allow user to specify GDK_ROOT
if(GDK_ROOT)
    set(GDK_ROOT_PATH "${GDK_ROOT}")
endif()

# Find GDK headers
find_path(GDK_INCLUDE_DIR
    NAMES XGameRuntime.h
    PATHS
        ${GDK_ROOT_PATH}/GRDK/gamekit/include
        ${GDK_ROOT_PATH}/GXDK/gamekit/include
    NO_DEFAULT_PATH
)

# Find GDK libraries based on architecture and configuration
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(GDK_ARCH "x64")
else()
    set(GDK_ARCH "x86")
endif()

find_library(GDK_LIBRARY
    NAMES xgameruntime.lib
    PATHS
        ${GDK_ROOT_PATH}/GRDK/gamekit/lib/amd64
        ${GDK_ROOT_PATH}/GXDK/gamekit/lib/amd64
    NO_DEFAULT_PATH
)

# Handle the QUIETLY and REQUIRED arguments
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GDK
    REQUIRED_VARS GDK_INCLUDE_DIR GDK_LIBRARY GDK_ROOT_PATH
    VERSION_VAR GDK_VERSION
)

if(GDK_FOUND)
    set(GDK_INCLUDE_DIRS ${GDK_INCLUDE_DIR})
    set(GDK_LIBRARIES ${GDK_LIBRARY})
    
    # Create imported target
    if(NOT TARGET Microsoft::GDK)
        add_library(Microsoft::GDK UNKNOWN IMPORTED)
        set_target_properties(Microsoft::GDK PROPERTIES
            IMPORTED_LOCATION "${GDK_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${GDK_INCLUDE_DIR}"
        )
    endif()
endif()

mark_as_advanced(GDK_INCLUDE_DIR GDK_LIBRARY)