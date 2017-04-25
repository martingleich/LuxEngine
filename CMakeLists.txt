cmake_minimum_required(VERSION 2.8.2)
project(LuxEngine)

# These values will appear in the source code
set(LuxEngine_VERSION_MAJOR 0)
set(LuxEngine_VERSION_MINOR 1)

# TODO It would be better if the user could specify 
# LuxEngine_COMPILE_WITH_D3D9 directly
if(WIN32)
	set(LuxEngine_OS_WINDOWS true)
	set(LuxEngine_OS_LINUX false)
	set(LuxEngine_COMPILE_WITH_D3D9 true)
	set(LuxEngine_COMPILE_WITH_D3DX_IMAGE_LOADER true)
	set(LuxEngine_COMPILE_WITH_RAW_INPUT true)
elseif(UNIX AND NOT APPLE)
	set(LuxEngine_OS_WINDOWS false)
	set(LuxEngine_OS_LINUX true)
	set(LuxEngine_COMPILE_WITH_D3D9 false)
	set(LuxEngine_COMPILE_WITH_D3DX_IMAGE_LOADER false)
	set(LuxEngine_COMPILE_WITH_RAW_INPUT false)
endif()

configure_file(
	"${PROJECT_SOURCE_DIR}/inc/LuxConfig.h.in"
	"${PROJECT_SOURCE_DIR}/inc/LuxConfig.h"
	)

add_subdirectory(external/format)
add_subdirectory(external/libpng)
add_subdirectory(external/libtga)
add_subdirectory(external/zlib)

add_subdirectory(testing/UnitTest)

# Always include inc!
include_directories("${PROJECT_SOURCE_DIR}/inc")
include_directories("${PROJECT_SOURCE_DIR}/src")

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

# TODO cmake should already know these libs (we are calling add_dependencies)
# for them later ...
set(USED_LIBS ${USED_LIBS} format zlib libtga libpng)

# Add plattform dependend libs and compiler-flags
if(MSVC)
	add_definitions(-std=c++11 -DLUX_EXPORT -DUNICODE -D_UNICODE -D_CRT_SECURE_NO_WARNINGS -FI"LuxConfig.h")
	set(USED_LIBS ${USED_LIBS} d3d9 d3dx9 Hid)
	link_directories(${PROJECT_SOURCE_DIR}/external/d3d9/x86/)
	set(SRCS ${SRCS} "${PROJECT_SOURCE_DIR}/tmp/$(Platform)$(Configuration).def")
else()
	add_definitions(-std=c++11 -Wall -DLUX_EXPORT -DUNICODE -D_UNICODE -DNDEBUG -fPIC -include "LuxConfig.h")
endif()

add_library(LuxEngine SHARED ${SRCS} ${INCS})
add_dependencies(LuxEngine format zlib libpng libtga)
target_link_libraries(LuxEngine ${USED_LIBS})

# Add tests
enable_testing()
add_test(NAME BasicTest COMMAND UnitTest)

if(WIN32)
	# TODO Ugly hacks to create the def file ...
	add_custom_command(
		TARGET LuxEngine
		PRE_BUILD
		COMMAND "${PROJECT_SOURCE_DIR}/scripts/ValidateDependencies.bat"
		COMMENT "Validate dependencies"
	)
	set_source_files_properties(
		"${PROJECT_SOURCE_DIR}/tmp/$(Platform)$(Configuration).def"
		PROPERTIES GENERATED TRUE
	)
	add_custom_command(
		TARGET LuxEngine
		PRE_LINK
		COMMAND "${PROJECT_SOURCE_DIR}/scripts/GenerateDef.bat" "${PROJECT_SOURCE_DIR}/lib/$(Platform)/$(Configuration)" "${PROJECT_SOURCE_DIR}/export.txt" "${PROJECT_SOURCE_DIR}/tmp/$(Platform)$(Configuration).def"
		COMMENT "Build .def file"
	)
endif()
