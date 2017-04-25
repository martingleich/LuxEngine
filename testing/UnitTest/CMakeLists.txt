# Let cmake look for all needed files
file(GLOB_RECURSE UNITTEST_INCS "src/*.h")
file(GLOB_RECURSE UNITTEST_SRCS "src/*.cpp")

include_directories("${PROJECT_SOURCE_DIR}/inc")
include_directories("${PROJECT_SOURCE_DIR}/testing/UnitTest/src")
link_directories(${PROJECT_SOURCE_DIR}/external/d3d9/x86/)

# Add plattform dependend libs and compiler-flags
if(MSVC)
	add_definitions(-DUNICODE -D_UNICODE -D_CRT_SECURE_NO_WARNINGS)
else()
	add_definitions(-std=c++11 -Wall -DUNICODE -D_UNICODE -DNDEBUG -fpermissive)
endif()

add_executable(UnitTest ${UNITTEST_SRCS} ${UNITTEST_INCS})
add_dependencies(UnitTest LuxEngine)
target_link_libraries(UnitTest LuxEngine)

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
assign_source_group(${UNITTEST_SRCS})
assign_source_group(${UNITTEST_INCS})
