cmake_minimum_required(VERSION 2.8.2)
project(dxerr)

set(SRCS 
	"dxerr.cpp"
	)
	
set(INCS
	"dxerr.h"
	)

add_library(dxerr ${SRCS} ${INCS})

include_directories("${PROJECT_SOURCE_DIR}")

if(WIN32)
elseif(UNIX AND NOT APPLE)
	add_definitions(-fPIC) # Only needed if compiling for Linux
endif()
	
if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
	add_definitions(-std=c++11 -Wall -DNDEBUG -pedantic)
endif()

# Create the filters for visual studio

function(assign_source_group)
	foreach(_source in ITEMS ${ARGN})
		if(IS_ABSOLUTE "${_source}")
			file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
		else()
			set(_source_rel "${_source}")
		endif()
		get_filename_component(_source_path "${_source_rel}" PATH)
		string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
		source_group("${_source_path_msvc}" FILES "${_source}")
	endforeach()
endfunction(assign_source_group)

assign_source_group(${FORMAT_SRCS})
assign_source_group(${FORMAT_INCS})