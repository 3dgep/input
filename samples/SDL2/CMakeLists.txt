cmake_minimum_required(VERSION 3.12...3.31)

add_executable(sample_SDL2 main.cpp stb_image.h ../../.clang-format)
target_link_libraries(sample_SDL2 PRIVATE input::SDL2 SDL2::SDL2main)

# Copy SDL2.dll to output directory after building
add_custom_command(TARGET sample_SDL2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL2::SDL2>
        $<TARGET_FILE_DIR:sample_SDL2>
)