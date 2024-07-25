#
# ocr - Copyright (C) 2024 P. Milovic
#
# See the LICENSE file for license information

# append to include directories
#list(APPEND OCR_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})
#list(APPEND OCR_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/src")
#list(APPEND OCR_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/lib")
#list(APPEND OCR_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/include")

# append library
list(APPEND OCR_LIBRARIES ocr)

# add sources
file(GLOB TEST_SRC *.cpp)

# binary target
foreach(TESTFILE ${TEST_SRC})
	get_filename_component(FILENAME ${TESTFILE} NAME_WE)
	#add_executable(${FILENAME} "${FILENAME}.cpp" ${OCR_SRC})
	add_executable(${FILENAME} "${FILENAME}.cpp")
	target_include_directories(${FILENAME} PRIVATE ${OCR_INCLUDE_DIRS})
	target_link_libraries(${FILENAME} PRIVATE ${OCR_LIBRARIES})
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
