#
# ocr - Copyright (C) 2024 P. Milovic
#
# See the LICENSE file for license information

# cmake options
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
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# define dependency vars
set(OCR_INCLUDE_DIRS "")
set(OCR_LIBRARIES "")

# FetchContent
#include(FetchContent)

# Tesseract
set(Leptonica_DIR "${PROJECT_SOURCE_DIR}/share/lib/cmake/leptonica")
set(Tesseract_DIR "${PROJECT_SOURCE_DIR}/share/lib/cmake/tesseract")
find_package(Tesseract REQUIRED)
list(APPEND OCR_INCLUDE_DIRS ${Tesseract_INCLUDE_DIRS})
list(APPEND OCR_LIBRARIES "Tesseract::libtesseract")

# OpenCV
set(OpenCV_DIR "${PROJECT_SOURCE_DIR}/share/lib/cmake/opencv4")
find_package(OpenCV REQUIRED)
list(APPEND OCR_INCLUDE_DIRS ${OpenCV_INCLUDE_DIRS})
list(APPEND OCR_LIBRARIES ${OpenCV_LIBS})

# Threads
#find_package(Threads REQUIRED)
#list(APPEND OCR_LIBRARIES "Threads::Threads")

# random linker stuff
#list(APPEND OCR_LIBRARIES "stdc++fs")

# add sources and headers
file(
	GLOB
	OCR_SRC
	CONFIGURE_DEPENDS
	"${PROJECT_SOURCE_DIR}/src/*.cpp"
	"${PROJECT_SOURCE_DIR}/src/ops/*.cpp")

file(
	GLOB
	OCR_HDR
	CONFIGURE_DEPENDS
	"${PROJECT_SOURCE_DIR}/src/*.h"
	"${PROJECT_SOURCE_DIR}/src/ops/*.h")

list(
	APPEND
	OCR_INCLUDE_DIRS 
	"${PROJECT_SOURCE_DIR}/src"
	"${PROJECT_SOURCE_DIR}/src/ops")

# main library target
add_library (ocr STATIC)
set_property(TARGET ocr
	PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
target_include_directories(ocr PUBLIC ${OCR_INCLUDE_DIRS})
target_link_libraries(ocr PRIVATE ${OCR_LIBRARIES})
target_link_options(ocr
	PRIVATE
	-Wl,--whole-archive
	${OCR_LIBRARIES}
	-Wl,--no-whole-archive
	)
target_sources(ocr PRIVATE ${OCR_SRC})
target_sources(ocr PUBLIC FILE_SET HEADERS FILES ${OCR_HDR})

# install targets
install(TARGETS ocr DESTINATION "${PROJECT_SOURCE_DIR}/lib")
install(FILES ${OCR_HDR} DESTINATION "${PROJECT_SOURCE_DIR}/include")

# enable testing
include(CTest)

if(CMAKE_BUILD_TYPE MATCHES Debug)
	add_subdirectory(test/src)
endif()

# documentation targets
#if(CMAKE_BUILD_TYPE MATCHES "Release")
#	add_subdirectory(doc)
#endif()
