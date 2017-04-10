# Let cmake look for all needed files
file(GLOB_RECURSE FORMAT_INCS "format/*.h")
file(GLOB_RECURSE FORMAT_SRCS "format/*.cpp")

# Always include the inc folder!
include_directories("inc")

add_definitions(-std=c++11)
add_library(format ${FORMAT_SRCS} ${FORMAT_INCS})

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
assign_source_group(${FORMAT_SRCS})
assign_source_group(${FORMAT_INCS})

