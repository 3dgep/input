cmake_minimum_required(VERSION 3.12...3.31)

add_executable(sample_SDL3 main.cpp ../../.clang-format)
target_link_libraries(sample_SDL3 PRIVATE input::SDL3)
