cmake_minimum_required(VERSION 3.12...3.31)

add_executable(sample_GDK main.cpp ../../.clang-format)
target_link_libraries(sample_GDK PRIVATE input::GDK)
target_compile_features(sample_GDK PUBLIC cxx_std_23)

