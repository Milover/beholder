#
# beholder - Copyright (C) 2024 P. Milovic
#
# See the LICENSE file for license information

# append to include directories
#list(APPEND BEHOLDER_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})
#list(APPEND BEHOLDER_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/src")
#list(APPEND BEHOLDER_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/lib")
#list(APPEND BEHOLDER_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/include")

# add sources
file(GLOB TEST_SRC "*.cpp")

# camera tests are enabled only on Linux
if (NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
	list(FILTER TEST_SRC EXCLUDE REGEX ".*camera.*\.cpp")
endif()

# set up the test binary
foreach(TESTFILE ${TEST_SRC})
	get_filename_component(FILENAME ${TESTFILE} NAME_WE)
	add_executable(${FILENAME} "${FILENAME}.cpp")
	target_include_directories(${FILENAME} PRIVATE ${BEHOLDER_INCLUDE_DIRS})
	target_link_libraries(${FILENAME} PRIVATE beholder)

	# camera tests are enabled on Linux
	if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
		target_include_directories(${FILENAME} PRIVATE ${BEHOLDER_CAMERA_INCLUDE_DIRS})
		target_link_libraries(${FILENAME}
			PRIVATE
			beholder_camera
			${BEHOLDER_CAMERA_LIBRARIES}
		)
	endif()

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
