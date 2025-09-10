cmake_minimum_required( VERSION 3.12...3.31 )

option( INPUT_BUILD_SAMPLES "Build Input samples." ON )

set( INPUT_VERSION_MAJOR 0 )
set( INPUT_VERSION_MINOR 0 )
set( INPUT_VERSION_PATCH 1 )
set( INPUT_VERSION ${INPUT_VERSION_MAJOR}.${INPUT_VERSION_MINOR}.${INPUT_VERSION_PATCH})

# Use solution folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project( Input VERSION ${INPUT_VERSION} LANGUAGES C CXX )

# Enable multi-processor builds in MSVC.
if(MSVC)
    add_compile_options(/MP)
endif(MSVC)

