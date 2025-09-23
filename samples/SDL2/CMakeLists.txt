cmake_minimum_required(VERSION 3.12...3.31)

set(SDL2TTF_VENDORED OFF CACHE BOOL "" FORCE)
set(SDL2TTF_FREETYPE_VENDORED OFF CACHE BOOL "" FORCE)

include(FetchContent)

if(NOT TARGET freetype)
    # Fetch freetype from GitHub
    FetchContent_Declare(
        freetype
        GIT_REPOSITORY https://github.com/freetype/freetype.git
        GIT_TAG master
    )
    FetchContent_MakeAvailable(freetype)
endif()

# Fetch SDL_ttf from GitHub
FetchContent_Declare(
    SDL2_ttf
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
    GIT_TAG release-2.24.0
)
FetchContent_MakeAvailable(SDL2_ttf)

set_target_properties(SDL2_ttf PROPERTIES FOLDER "externals/SDL2")
set_target_properties(freetype PROPERTIES FOLDER "externals/SDL2")

add_executable(sample_SDL2 main.cpp stb_image.h ../../.clang-format)
target_link_libraries(sample_SDL2 PRIVATE input::SDL2 SDL2::SDL2main)

set(DEPENDENCY_FILES "")

if(TARGET SDL2_ttf::SDL2_ttf)
    target_link_libraries(sample_SDL2 PRIVATE SDL2_ttf::SDL2_ttf)
    list(APPEND DEPENDENCY_FILES $<TARGET_FILE:SDL2::SDL2>)
elseif(TARGET SDL2_ttf::SDL2_ttf-static)
    target_link_libraries(sample_SDL2 PRIVATE SDL2_ttf::SDL2_ttf-static)
    list(APPEND DEPENDENCY_FILES $<TARGET_FILE:SDL2_ttf::SDL2_ttf>)
endif()

# Copy File dependencies to the output folder.
add_custom_command(TARGET sample_SDL2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${DEPENDENCY_FILES}
    $<TARGET_FILE_DIR:sample_SDL2>
)
