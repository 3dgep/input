cmake_minimum_required(VERSION 3.10...3.31)

function( add_sample DIR_NAME )
	if( NOT INPUT_TEST_INCLUDED )
		add_subdirectory( input )
		set( INPUT_TEST_INCLUDED TRUE PARENT_SCOPE)
	endif()
	add_subdirectory( ${DIR_NAME} )
	if( TARGET sample_${DIR_NAME} )
	set_target_properties( input_test sample_${DIR_NAME}
		PROPERTIES 
			FOLDER samples
			VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
	endif()
endfunction()

if(INPUT_USE_GDK AND TARGET Microsoft::GDK)
    add_sample(GDK)
endif()

if(INPUT_USE_GLFW AND TARGET glfw::glfw)
    add_sample(GLFW)
endif()

if(INPUT_USE_SDL2 AND TARGET SDL2::SDL2)
    add_sample(SDL2)
endif()

if(INPUT_USE_SDL3 AND TARGET SDL3::SDL3)
    add_sample(SDL3)
endif()

if(INPUT_USE_WIN32 AND WIN32)
    add_sample(Win32)
endif()
