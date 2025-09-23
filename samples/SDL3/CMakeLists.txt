cmake_minimum_required(VERSION 3.12...3.31)

add_executable(sample_SDL3 main.cpp ../../.clang-format)
target_link_libraries(sample_SDL3 PRIVATE input::SDL3)

# Copy SDL3.dll to output directory after building
add_custom_command(TARGET sample_SDL3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL3::SDL3>
        $<TARGET_FILE_DIR:sample_SDL3>
)