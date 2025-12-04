cmake_minimum_required(VERSION 4.0...4.2)

macro( add_sample DIR_NAME )
	if( NOT INPUT_TEST_INCLUDED )
		add_subdirectory( input )
		set( INPUT_TEST_INCLUDED TRUE )
	endif()
	add_subdirectory( ${DIR_NAME} )
	if( TARGET sample_${DIR_NAME} )
	set_target_properties( input_test sample_${DIR_NAME}
		PROPERTIES 
			FOLDER samples
			DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
	set_property( DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT sample_${DIR_NAME})
	endif()
endmacro()

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
