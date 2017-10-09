set(SRCS 
	"src/main.cpp"
)
set(INCS
	"src/Context.h"
)

include_directories("${PROJECT_SOURCE_DIR}/inc")
include_directories("${PROJECT_SOURCE_DIR}/testing/MaterialTest/src")
link_directories(${PROJECT_SOURCE_DIR}/external/d3d9/x86/)

# Add plattform dependend libs and compiler-flags
if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	
else()
	add_definitions(-std=c++14 -Wall -DUNICODE -D_UNICODE -DNDEBUG)
endif()

add_executable(MaterialTest ${SRCS} ${INCS})
target_link_libraries(MaterialTest LuxEngine)

# http://stackoverflow.com/questions/31422680/how-to-set-visual-studio-filters-for-nested-sub-directory-using-cmake
function(assign_source_group)
	foreach(_source in ITEMS ${ARGN})
		if(IS_ABSOLUTE "${_source}")
			file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
		else()
			set(_source_rel "${_source}")
		endif()
		get_filename_component(_source_path "${_source_rel}" PATH)
		String(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
		source_group("${_source_path_msvc}" FILES "${_source}")
	endforeach()
endfunction(assign_source_group)

# Create the filters for visual studio
assign_source_group(${SRCS})
assign_source_group(${INCS})

add_custom_command(
	TARGET MaterialTest
	POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        $<TARGET_FILE:LuxEngine>      # <--this is in-file
        $<TARGET_FILE_DIR:MaterialTest>)               # <--this is out-file path
