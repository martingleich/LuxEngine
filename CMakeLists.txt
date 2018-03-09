cmake_minimum_required(VERSION 2.8.2)
project(LuxEngine)

# These values will appear in the source code
set(LuxEngine_VERSION_MAJOR 0)
set(LuxEngine_VERSION_MINOR 1)

option(FORMAT_EXCEPTIONS "Throw exceptions on invalid format" ON)
option(FORMAT_ERROR_TEXT "Write error string if single placeholder fails" OFF)

if(WIN32)
	set(LuxEngine_OS_WINDOWS true)
	set(LuxEngine_OS_LINUX false)
	option(LUX_COMPILE_WITH_D3D9 "" ON)
	option(LUX_COMPILE_WITH_D3DX_IMAGE_LOADER "" ON)
	option(LUX_COMPILE_WITH_RAW_INPUT "" ON)
elseif(UNIX AND NOT APPLE)
	set(LuxEngine_OS_WINDOWS false)
	set(LuxEngine_OS_LINUX true)
	option(LUX_COMPILE_WITH_D3D9 "" OFF)
	option(LUX_COMPILE_WITH_D3DX_IMAGE_LOADER "" OFF)
	option(LUX_COMPILE_WITH_RAW_INPUT "" OFF)
endif()

configure_file(
	"${PROJECT_SOURCE_DIR}/inc/LuxConfig.h.in"
	"${PROJECT_SOURCE_DIR}/inc/LuxConfig.h"
	)

add_subdirectory(external/libpng)
add_subdirectory(external/libtga)
add_subdirectory(external/zlib)
add_subdirectory(external/utf8proc)
add_subdirectory(external/tinyobjloader)

add_subdirectory(testing/UnitTest)
add_subdirectory(testing/MaterialTest)

if(WIN32)
	#add_subdirectory(external/FilterExportFiles)
	add_subdirectory(external/dxerr)
endif()

# Always include inc!
include_directories("${PROJECT_SOURCE_DIR}/inc")
include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/external")

# I'm not going to list all cpp files manually ...
file(GLOB_RECURSE SRCS_CPP "src/*.cpp")
file(GLOB_RECURSE SRCS_H "src/*.h")
set(SRCS ${SRCS_CPP} ${SRCS_H})
file(GLOB_RECURSE INCS "inc/*.h")

# Create the Porject and set link files
add_library(LuxEngine SHARED ${SRCS} ${INCS} )
target_link_libraries(LuxEngine zlib libtga libpng utf8proc tinyobjloader)

if(NOT FORMAT_EXCEPTIONS)
	add_definitions(-DFORMAT_NO_EXCEPTIONS)
endif()

if(FORMAT_ERROR_TEXT)
	add_definitions(-DFORMAT_ERROR_TEXT)
endif()

# Add plattform dependend libs and compiler-flags
if(MSVC)
	add_definitions(-DLUX_EXPORT -DUNICODE -D_UNICODE -D_CRT_SECURE_NO_WARNINGS -FI"LuxConfig.h" -W4)
	
	# Check for x64
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		find_library(D3DLIB d3d9 HINTS ${PROJECT_SOURCE_DIR}/external/d3d9/x64/)
		find_library(D3DXLIB d3dx9 HINTS ${PROJECT_SOURCE_DIR}/external/d3d9/x64/)
	else()
		find_library(D3DLIB d3d9 HINTS ${PROJECT_SOURCE_DIR}/external/d3d9/x86/)
		find_library(D3DXLIB d3dx9 HINTS ${PROJECT_SOURCE_DIR}/external/d3d9/x86/)
	endif()
	target_link_libraries(LuxEngine ${D3DLIB} ${D3DXLIB} Hid dxerr)

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /w34296 ") #'operator': expression is always false
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /w34062 ") # enumerator 'identifier' in a switch of enum 'enumeration' is not handled
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /w34264 ") # 'virtual_function': no override available for virtual member function from base 'class'; function is hidden
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /w34265 ") # 'class': class has virtual functions, but destructor is not virtual
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /w34928 ") # illegal copy-initialization; more than one user-defined conversion has been implicitly applied
else()
	if(NOT WIN32)
		add_definitions(-fPIC)
	endif()
	add_definitions(-std=c++14 -Wall -DLUX_EXPORT -DUNICODE -D_UNICODE -DNDEBUG -include "LuxConfig.h")
endif()

# Add tests
enable_testing()
add_test(NAME BasicTest COMMAND UnitTest)

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
