cmake_minimum_required(VERSION 3.12...3.31)

add_library( input_test STATIC input_test.cpp input_test.hpp )
add_library( input::test ALIAS input_test )
target_include_directories( input_test 
	PUBLIC . 
	PRIVATE "../../inc"
)

target_compile_features( input_test PUBLIC cxx_std_20)