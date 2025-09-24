cmake_minimum_required(VERSION 3.12...3.31)

add_executable(sample_SDL2 main.cpp stb_image.h ../../.clang-format)
target_link_libraries(sample_SDL2 PRIVATE input::SDL2 SDL2::SDL2main)

set(DEPENDENCY_FILES $<TARGET_FILE:SDL2::SDL2> $<TARGET_FILE:SDL2::SDL2main>)

if(TARGET SDL2_ttf::SDL2_ttf)
    target_link_libraries(sample_SDL2 PRIVATE SDL2_ttf::SDL2_ttf)
    list(APPEND DEPENDENCY_FILES $<TARGET_FILE:SDL2_ttf::SDL2_ttf>)
elseif(TARGET SDL2_ttf::SDL2_ttf-static)
    target_link_libraries(sample_SDL2 PRIVATE SDL2_ttf::SDL2_ttf-static)
endif()

# Copy File dependencies to the output folder.
add_custom_command(TARGET sample_SDL2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${DEPENDENCY_FILES}
    $<TARGET_FILE_DIR:sample_SDL2>
)
