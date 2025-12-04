cmake_minimum_required(VERSION 3.12...3.31)

add_executable(sample_GLFW main.cpp ../../.clang-format)
target_link_libraries(sample_GLFW PRIVATE input::GLFW input::test)

target_compile_features(sample_GLFW PRIVATE cxx_std_23)

# Copy File dependencies to the output folder.
add_custom_command(TARGET sample_GLFW POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:glfw::glfw>
    $<TARGET_FILE_DIR:sample_GLFW>
)