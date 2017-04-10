cmake_minimum_required(VERSION 2.8.2)
project(LuxEngine)

# These values will appear in the source code
set(LuxEngine_VERSION_MAJOR 0)
set(LuxEngine_VERSION_MINOR 1)
configure_file(
	"${PROJECT_SOURCE_DIR}/inc/LuxConfig.h.in"
	"${PROJECT_SOURCE_DIR}/inc/LuxConfig.h"
	)

# Always include inc!
include_directories("${PROJECT_SOURCE_DIR}/inc")
include_directories("${PROJECT_SOURCE_DIR}/src")

add_subdirectory(external/format)
add_subdirectory(external/libpng)
add_subdirectory(external/libtga)
add_subdirectory(external/zlib)

# I'm not going to list all cpp files manually ...
file(GLOB_RECURSE SRCS_CPP "src/*.cpp")
file(GLOB_RECURSE SRCS_H "src/*.h")
set(SRCS ${SRCS_CPP} ${SRCS_H})
file(GLOB_RECURSE INCS "inc/*.h")

# http://stackoverflow.com/questions/31422680/how-to-set-visual-studio-filters-for-nested-sub-directory-using-cmake
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

# Create the filters for visual studio
assign_source_group(${SRCS})
assign_source_group(${INCS})

link_directories("external/d3d9/x86")
set(USED_LIBS ${USED_LIBS} format zlib libtga libpng)

# Build the executable (with the corresponding libraries)
if(MSVC)
	add_definitions(-std=c++11 -DLUX_EXPORT -DUNICODE -D_UNICODE)
	set(USED_LIBS ${USED_LIBS} d3d9 d3dx9 Hid)
	link_directories("$(PROJECT_SOURCE_DIR}/external/d3d9/x86")
else()
	add_definitions(-std=c++11 -Wall -DLUX_EXPORT -DUNICODE -D_UNICODE)
endif()
add_library(LuxEngine SHARED ${SRCS} ${INCS})
target_link_libraries(LuxEngine ${USED_LIBS})
add_dependencies(LuxEngine format zlib libpng libtga)

