cmake_minimum_required(VERSION 3.10...3.31)

function( add_sample DIR_NAME )
	add_subdirectory( ${DIR_NAME} )
	set_target_properties( sample_${DIR_NAME}
		PROPERTIES 
			FOLDER samples
			VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
endfunction()

if(GDK_FOUND)
    add_sample(GDK)
endif()

if(GLFW_FOUND)
    add_sample(GLFW)
endif()

if(SDL2_FOUND)
    add_sample(SDL2)
endif()

if(SDL3_FOUND)
    add_sample(SDL3)
endif()

if(WIN32)
    add_sample(Win32)
endif()
