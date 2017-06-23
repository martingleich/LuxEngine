macro(ADD_PRECOMPILED_HEADER PrecompiledHeader PrecompiledSource SourcesVar)
  if(MSVC)
    get_filename_component(PrecompiledBasename ${PrecompiledHeader} NAME_WE)
    set(PrecompiledBinary "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${PrecompiledBasename}.pch")
    set(Sources ${${SourcesVar}})

    set_source_files_properties(${PrecompiledSource}
                                PROPERTIES COMPILE_FLAGS "/Yc\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\""
                                           OBJECT_OUTPUTS "${PrecompiledBinary}")
    set_source_files_properties(${Sources}
                                PROPERTIES COMPILE_FLAGS "/Yu\"${PrecompiledHeader}\" /FI\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\""
                                           OBJECT_DEPENDS "${PrecompiledBinary}")  
    # Add precompiled source file to sources
    list(APPEND ${SourcesVar} ${PrecompiledSource})
  endif(MSVC)
endmacro(ADD_PRECOMPILED_HEADER)

set(UNITTEST_SRCS 
	"src/main.cpp"
	"src/UnitTest.cpp"
	"src/UnitTestEx.cpp"
	# Tests
	"src/Tests/AlgorithmTest.cpp"
	"src/Tests/ArrayTest.cpp"
	"src/Tests/ColorTest.cpp"
	"src/Tests/FileSystemTest.cpp"
	"src/Tests/FormatTest.cpp"
	"src/Tests/HashMapTest.cpp"
	"src/Tests/MatrixTest.cpp"
	"src/Tests/PathTest.cpp"
	"src/Tests/QuaternionTest.cpp"
	"src/Tests/StringConverterTest.cpp"
	"src/Tests/StringTest.cpp"
	"src/Tests/TransformationTest.cpp"
	"src/Tests/UTF8Test.cpp"
)

set(UNITTEST_INCS
	"src/stdafx.h"
	"src/UnitTest.h"
	"src/UnitTestEx.h"
	)

include_directories("${PROJECT_SOURCE_DIR}/inc")
include_directories("${PROJECT_SOURCE_DIR}/testing/UnitTest/src")
link_directories(${PROJECT_SOURCE_DIR}/external/d3d9/x86/)

# Add plattform dependend libs and compiler-flags
if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	
else()
	add_definitions(-std=c++11 -Wall -DUNICODE -D_UNICODE -DNDEBUG)
endif()

# Not using full path to stdafx.h since the include in the cpp must be equal to this string.
ADD_PRECOMPILED_HEADER("stdafx.h" "src/stdafx.cpp" UNITTEST_SRCS)

add_executable(UnitTest ${UNITTEST_SRCS} ${UNITTEST_INCS})
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

add_custom_command(
	TARGET UnitTest
	POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        $<TARGET_FILE:LuxEngine>      # <--this is in-file
        $<TARGET_FILE_DIR:UnitTest>)               # <--this is out-file path
