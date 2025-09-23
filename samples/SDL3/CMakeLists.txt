cmake_minimum_required(VERSION 3.12...3.31)

set(SDLTTF_FREETYPE_VENDORED OFF CACHE BOOL "" FORCE)

include(FetchContent)

if( NOT TARGET freetype )
# Fetch freetype from GitHub
FetchContent_Declare(
    freetype
    GIT_REPOSITORY https://github.com/freetype/freetype.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(freetype)
endif()

# Fetch SDL_ttf from GitHub
FetchContent_Declare(
    SDL3_ttf
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
    GIT_TAG        release-3.2.2
)
FetchContent_MakeAvailable(SDL3_ttf)

add_executable(sample_SDL3 main.cpp ../../.clang-format)
target_link_libraries(sample_SDL3 PRIVATE input::SDL3 SDL3_ttf::SDL3_ttf)

# Copy SDL3.dll to output directory after building
add_custom_command(TARGET sample_SDL3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL3::SDL3>
        $<TARGET_FILE_DIR:sample_SDL3>
)