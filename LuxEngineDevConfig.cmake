cmake_minimum_required(VERSION 2.8)

set(BUILD_DIR ${CMAKE_CURRENT_LIST_DIR}/build)
set(INC_DIR ${CMAKE_CURRENT_LIST_DIR}/inc)

add_library(LuxEngine SHARED IMPORTED GLOBAL)
set_target_properties(LuxEngine PROPERTIES
	IMPORTED_LOCATION "${BUILD_DIR}/Release/LuxEngine.dll"
	IMPORTED_LOCATION_DEBUG "${BUILD_DIR}/Debug/LuxEngine.dll"
	IMPORTED_LOCATION_RELEASE "${BUILD_DIR}/Release/LuxEngine.dll"
	IMPORTED_LOCATION_RELWITHDEBINFO "${BUILD_DIR}/RelWithDebInfo/LuxEngine.dll"
	
	IMPORTED_IMPLIB "${BUILD_DIR}/Release/LuxEngine.lib"
	IMPORTED_IMPLIB_DEBUG "${BUILD_DIR}/Debug/LuxEngine.lib"
	IMPORTED_IMPLIB_RELEASE "${BUILD_DIR}/Release/LuxEngine.lib"
	IMPORTED_IMPLIB_RELWITHDEBINFO "${BUILD_DIR}/RelWithDebInfo/LuxEngine.lib"
	
	INTERFACE_INCLUDE_DIRECTORIES "${INC_DIR};${BUILD_DIR}")
	
function(copy_lux_dll target)
	add_custom_command(
		TARGET ${target}
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
			$<TARGET_FILE:LuxEngine>
			$<TARGET_FILE_DIR:${target}>)               # <--this is out-file path
endfunction(copy_lux_dll)