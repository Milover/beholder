#
# camera - Copyright (C) 2024 P. Milovic
#
# See the LICENSE file for license information

# compiler flags
add_compile_options(
		"-Wall" "-Wextra" "-Wshadow" "-pedantic"
)

# append to include directories
list(APPEND CAMERA_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})
list(APPEND CAMERA_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/src")
list(APPEND CAMERA_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/lib")
list(APPEND CAMERA_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/include")

# append library
list(APPEND CAMERA_LIBRARIES camera)

# add sources
file(GLOB TEST_SRC *.cpp)

# binary target
foreach(TESTFILE ${TEST_SRC})
	get_filename_component(FILENAME ${TESTFILE} NAME_WE)
	#add_executable(${FILENAME} "${FILENAME}.cpp" ${CAMERA_SRC})
	add_executable(${FILENAME} "${FILENAME}.cpp")
	target_include_directories(${FILENAME} PRIVATE ${CAMERA_INCLUDE_DIRS})
	target_link_libraries(${FILENAME} PRIVATE ${CAMERA_LIBRARIES})
	set_target_properties(${FILENAME}
		PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/test/bin"
	)

	add_test(NAME ${FILENAME}
		COMMAND ${FILENAME}
		WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/test/bin"
	)
#	set_tests_properties(${FILENAME}
#		PROPERTIES
#		PASS_REGULAR_EXPRESSION "0"
#	)
endforeach()
