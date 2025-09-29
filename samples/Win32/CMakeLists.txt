cmake_minimum_required(VERSION 3.12...3.31)

add_executable(sample_Win32 WIN32 main.cpp ../../.clang-format)
target_link_libraries(sample_Win32 PRIVATE input::Win32 input::test)
