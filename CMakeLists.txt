# ocr - Copyright (C) 2024 P. Milovic
#
# See the LICENSE file for license information

cmake_minimum_required(VERSION 3.26)

# project name
project(ocr)

# build type
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug CACHE STRING
		"Choose type of build, options are: Debug Release"
		FORCE)
endif()

# install prefix
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})
endif()

# compiler settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_VERBOSE_MAKEFILE FALSE)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
#set(THREADS_PREFER_PTHREAD_FLAG ON)

# compiler flags
add_compile_options(
		"-Wall" "-Wextra" "-Wshadow" "-pedantic"
		"$<$<CONFIG:DEBUG>:-O0>"
		"$<$<CONFIG:RELEASE>:-march=native>"
)

# output paths
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

# search paths
list(APPEND CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/src")
list(APPEND CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/include")
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/tools/cmake")

# define dependency vars
set(OCR_INCLUDE_DIRS "")
set(OCR_LIBRARIES "")

# PkgConfig
find_package(PkgConfig REQUIRED)

# Tesseract
pkg_check_modules(tesseract REQUIRED IMPORTED_TARGET tesseract)
list(APPEND OCR_LIBRARIES "PkgConfig::tesseract")

# Leptonica
pkg_check_modules(leptonica REQUIRED IMPORTED_TARGET lept)
list(APPEND OCR_LIBRARIES "PkgConfig::leptonica")
#find_package(Leptonica REQUIRED)
#list(APPEND OCR_INCLUDE_DIRS ${Leptonica_INCLUDE_DIRS})
#list(APPEND OCR_LIBRARIES ${Leptonica_LIBRARIES})

# OpenCV

# Threads
#find_package(Threads REQUIRED)
#list(APPEND OCR_LIBRARIES "Threads::Threads")

# random linker stuff
#list(APPEND OCR_LIBRARIES "stdc++fs")

# add sources macro
macro(append_ocr_src DIRNAME FILES)
	foreach(FILE ${FILES})
		list(APPEND LIST ${DIRNAME}/${FILE})
	endforeach()
	set(OCR_SRC ${OCR_SRC};${LIST} PARENT_SCOPE)
	set(OCR_INCLUDE_DIRS ${OCR_INCLUDE_DIRS};${DIRNAME} PARENT_SCOPE)
endmacro()

# enable testing
include(CTest)

# subdirectories
add_subdirectory(src)

#if(CMAKE_BUILD_TYPE MATCHES Debug)
#	add_subdirectory(test/src)
#endif()

# binary target
add_executable (ocr "${PROJECT_SOURCE_DIR}/src/Main.cpp" ${OCR_SRC})
target_include_directories(ocr PRIVATE ${OCR_INCLUDE_DIRS})
target_link_libraries(ocr PRIVATE ${OCR_LIBRARIES})

# main library target (for tests)
#add_library (libocr SHARED ${OCR_SRC})
#set_target_properties (libocr PROPERTIES PREFIX "")
#target_include_directories(libocr PUBLIC ${OCR_INCLUDE_DIRS})
#target_link_libraries(libocr ${OCR_LIBRARIES})

# install targets
install(TARGETS ocr DESTINATION "${PROJECT_SOURCE_DIR}/bin")

# documentation targets
#if(CMAKE_BUILD_TYPE MATCHES "Release")
#	add_subdirectory(doc)
#endif()
