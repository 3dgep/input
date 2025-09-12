# FindGLFW.cmake
# Locate GLFW library

# Try to find GLFW header
find_path(GLFW_INCLUDE_DIR 
    NAMES GLFW/glfw3.h
    HINTS
        ENV GLFW_ROOT
        ${GLFW_ROOT}
    PATHS
        /usr/local
        /usr
        /opt/local
        /opt
    PATH_SUFFIXES include
)

# Determine library suffix based on compiler and platform
if(MSVC)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(GLFW_LIB_SUFFIX "lib-vc2022/x64" "lib-vc2019/x64" "lib-vc2017/x64" "lib-mingw-w64")
    else()
        set(GLFW_LIB_SUFFIX "lib-vc2022/x86" "lib-vc2019/x86" "lib-vc2017/x86" "lib-mingw")
    endif()
else()
    set(GLFW_LIB_SUFFIX "lib" "lib64")
endif()

# Find GLFW library
find_library(GLFW_LIBRARY
    NAMES glfw glfw3 glfw3dll
    HINTS
        ENV GLFW_ROOT
        ${GLFW_ROOT}
    PATHS
        /usr/local
        /usr
        /opt/local
        /opt
    PATH_SUFFIXES ${GLFW_LIB_SUFFIX}
)

set(GLFW_LIBRARIES ${GLFW_LIBRARY})
set(GLFW_INCLUDE_DIRS ${GLFW_INCLUDE_DIR})

# Extract version from header if possible
if(GLFW_INCLUDE_DIR AND EXISTS "${GLFW_INCLUDE_DIR}/GLFW/glfw3.h")
    file(READ "${GLFW_INCLUDE_DIR}/GLFW/glfw3.h" GLFW_HEADER_CONTENT)
    string(REGEX MATCH "#define GLFW_VERSION_MAJOR[ \t]+([0-9]+)" _ ${GLFW_HEADER_CONTENT})
    set(GLFW_VERSION_MAJOR ${CMAKE_MATCH_1})
    string(REGEX MATCH "#define GLFW_VERSION_MINOR[ \t]+([0-9]+)" _ ${GLFW_HEADER_CONTENT})
    set(GLFW_VERSION_MINOR ${CMAKE_MATCH_1})
    string(REGEX MATCH "#define GLFW_VERSION_REVISION[ \t]+([0-9]+)" _ ${GLFW_HEADER_CONTENT})
    set(GLFW_VERSION_REVISION ${CMAKE_MATCH_1})
    set(GLFW_VERSION "${GLFW_VERSION_MAJOR}.${GLFW_VERSION_MINOR}.${GLFW_VERSION_REVISION}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLFW
    REQUIRED_VARS GLFW_LIBRARY GLFW_INCLUDE_DIR
    VERSION_VAR GLFW_VERSION
)

if(GLFW_FOUND AND NOT TARGET glfw)
    add_library(glfw UNKNOWN IMPORTED)
    set_target_properties(glfw PROPERTIES
        IMPORTED_LOCATION "${GLFW_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${GLFW_INCLUDE_DIR}"
    )
    
    # Add platform-specific dependencies
    if(APPLE)
        set_property(TARGET glfw APPEND PROPERTY
            INTERFACE_LINK_LIBRARIES "-framework Cocoa" "-framework IOKit" "-framework CoreVideo"
        )
    elseif(UNIX)
        set_property(TARGET glfw APPEND PROPERTY
            INTERFACE_LINK_LIBRARIES "${CMAKE_DL_LIBS}" "pthread" "X11"
        )
    elseif(WIN32)
        set_property(TARGET glfw APPEND PROPERTY
            INTERFACE_LINK_LIBRARIES "opengl32" "gdi32" "user32" "kernel32"
        )
    endif()
endif()

mark_as_advanced(GLFW_INCLUDE_DIR GLFW_LIBRARY)