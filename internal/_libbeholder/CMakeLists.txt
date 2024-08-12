#
# beholder - Copyright (C) 2024 P. Milovic
#
# See the LICENSE file for license information

# cmake options
cmake_minimum_required(VERSION 3.25)

# build type
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug CACHE STRING
		"Choose type of build, options are: Debug Release"
		FORCE)
endif()

# project name
project(beholder)

# restrict platform
if (NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
	message(FATAL_ERROR "Camera support can only be built on Linux")
endif()

# install prefix
#if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
#	set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})
#endif()

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

###############################################################################
# define dependency vars
set(BEHOLDER_INCLUDE_DIRS "")
set(BEHOLDER_LIBRARIES "")

set(BEHOLDER_CAMERA_INCLUDE_DIRS "")
set(BEHOLDER_CAMERA_LIBRARIES "")

# FetchContent
#include(FetchContent)

# we always build the main library statically, so we use static third-party
# libraries only
# Leptonica
set(Leptonica_DIR "${PROJECT_SOURCE_DIR}/share/lib/cmake/leptonica")
#find_package(Leptonica REQUIRED)
#list(APPEND BEHOLDER_INCLUDE_DIRS ${Leptonica_INCLUDE_DIRS})
#list(APPEND BEHOLDER_LIBRARIES ${Leptonica_LIBRARIES})

# Tesseract
set(Tesseract_DIR "${PROJECT_SOURCE_DIR}/share/lib/cmake/tesseract")
find_package(Tesseract REQUIRED)
list(APPEND BEHOLDER_INCLUDE_DIRS ${Tesseract_INCLUDE_DIRS})
list(APPEND BEHOLDER_LIBRARIES "Tesseract::libtesseract")

# OpenCV
set(OpenCV_DIR "${PROJECT_SOURCE_DIR}/share/lib/cmake/opencv4")
find_package(OpenCV REQUIRED)
list(APPEND BEHOLDER_INCLUDE_DIRS ${OpenCV_INCLUDE_DIRS})
list(APPEND BEHOLDER_LIBRARIES ${OpenCV_LIBS})

# pylon
find_package(pylon 7.5.0 REQUIRED)
list(APPEND BEHOLDER_CAMERA_INCLUDE_DIRS ${pylon_INCLUDE_DIRS})
list(APPEND BEHOLDER_CAMERA_LIBRARIES "pylon::pylon")

# Threads
#find_package(Threads REQUIRED)
#list(APPEND BEHOLDER_LIBRARIES "Threads::Threads")

# random linker stuff
#list(APPEND BEHOLDER_LIBRARIES "stdc++fs")

###############################################################################
# add main library sources and headers
file(
	GLOB
	BEHOLDER_SRC
	CONFIGURE_DEPENDS
	"${PROJECT_SOURCE_DIR}/src/*.cpp"
	"${PROJECT_SOURCE_DIR}/src/image/*.cpp"
	"${PROJECT_SOURCE_DIR}/src/image/ops/*.cpp"
	"${PROJECT_SOURCE_DIR}/src/ocr/*.cpp"
)

file(
	GLOB
	BEHOLDER_HDR
	CONFIGURE_DEPENDS
	"${PROJECT_SOURCE_DIR}/src/*.h"
	"${PROJECT_SOURCE_DIR}/src/image/*.h"
	"${PROJECT_SOURCE_DIR}/src/image/ops/*.h"
	"${PROJECT_SOURCE_DIR}/src/ocr/*.h"
)

list(
	APPEND
	BEHOLDER_INCLUDE_DIRS 
	"${PROJECT_SOURCE_DIR}/src"
	"${PROJECT_SOURCE_DIR}/src/image"
	"${PROJECT_SOURCE_DIR}/src/image/ops"
	"${PROJECT_SOURCE_DIR}/src/ocr"
)

# main library target
add_library(beholder STATIC)
set_target_properties(beholder
		PROPERTIES
		#CMAKE_POSITION_INDEPENDENT_CODE TRUE
		INTERPROCEDURAL_OPTIMIZATION TRUE
)
target_include_directories(beholder PRIVATE ${BEHOLDER_INCLUDE_DIRS})
target_link_libraries(beholder PRIVATE ${BEHOLDER_LIBRARIES})
target_sources(beholder PRIVATE ${BEHOLDER_SRC})
target_sources(beholder PUBLIC FILE_SET HEADERS FILES ${BEHOLDER_HDR})

# generate main library global header
set(INCLUDE_BEHOLDER_HDR "")
foreach(HEADER ${BEHOLDER_HDR})
	get_filename_component(HDR ${HEADER} NAME)
	set(INCLUDE_BEHOLDER_HDR "${INCLUDE_BEHOLDER_HDR}#include \"${HDR}\"\n")
endforeach()
configure_file(
	${CMAKE_SOURCE_DIR}/src/libbeholder.h.in
	${CMAKE_BINARY_DIR}/libbeholder.h
	@ONLY
)
list(APPEND BEHOLDER_HDR "${CMAKE_BINARY_DIR}/libbeholder.h")

# generate pkg-config
#configure_file(
#	${CMAKE_SOURCE_DIR}/beholder.pc.in
#	${CMAKE_BINARY_DIR}/beholder.pc
#	@ONLY
#)

# install targets
#install(TARGETS beholder LIBRARY DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")
#install(FILES ${BEHOLDER_HDR} DESTINATION "${CMAKE_INSTALL_PREFIX}/include/beholder")
install(TARGETS beholder ARCHIVE DESTINATION "${PROJECT_SOURCE_DIR}/share/lib")
install(FILES ${BEHOLDER_HDR} DESTINATION "${PROJECT_SOURCE_DIR}/share/include/beholder")

# install the pkg-config
#install(FILES "${CMAKE_BINARY_DIR}/beholder.pc"
#	DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig"
#)

###############################################################################
# add camera library sources and headers
file(
	GLOB
	BEHOLDER_CAMERA_SRC
	CONFIGURE_DEPENDS
	"${PROJECT_SOURCE_DIR}/src/*.cpp"
	"${PROJECT_SOURCE_DIR}/src/camera/*.cpp"
)

file(
	GLOB
	BEHOLDER_CAMERA_HDR
	CONFIGURE_DEPENDS
	"${PROJECT_SOURCE_DIR}/src/*.h"
	"${PROJECT_SOURCE_DIR}/src/camera/*.h"
)

list(
	APPEND
	BEHOLDER_CAMERA_INCLUDE_DIRS
	"${PROJECT_SOURCE_DIR}/src"
	"${PROJECT_SOURCE_DIR}/src/camera"
)

# camera library target
add_library(beholder_camera SHARED)
target_include_directories(beholder_camera PRIVATE ${BEHOLDER_CAMERA_INCLUDE_DIRS})
target_link_libraries(beholder_camera PRIVATE ${BEHOLDER_CAMERA_LIBRARIES})
target_sources(beholder_camera PRIVATE ${BEHOLDER_CAMERA_SRC})
target_sources(beholder_camera PUBLIC FILE_SET HEADERS FILES ${BEHOLDER_CAMERA_HDR})

# generate camera library global header
set(INCLUDE_BEHOLDER_CAMERA_HDR "")
foreach(HEADER ${BEHOLDER_CAMERA_HDR})
	get_filename_component(HDR ${HEADER} NAME)
	set(INCLUDE_BEHOLDER_CAMERA_HDR "${INCLUDE_BEHOLDER_CAMERA_HDR}#include \"${HDR}\"\n")
endforeach()
configure_file(
	${CMAKE_SOURCE_DIR}/src/libbeholder_camera.h.in
	${CMAKE_BINARY_DIR}/libbeholder_camera.h
	@ONLY
)
list(APPEND BEHOLDER_CAMERA_HDR "${CMAKE_BINARY_DIR}/libbeholder_camera.h")

# generate pkg-config
configure_file(
	${CMAKE_SOURCE_DIR}/beholder_camera.pc.in
	${CMAKE_BINARY_DIR}/beholder_camera.pc
	@ONLY
)

# install targets
install(TARGETS beholder_camera LIBRARY DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")
install(FILES ${BEHOLDER_CAMERA_HDR} DESTINATION "${CMAKE_INSTALL_PREFIX}/include/beholder/camera")

# install the pkg-config
install(FILES "${CMAKE_BINARY_DIR}/beholder_camera.pc"
	DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig"
)

###############################################################################
# enable testing in debug mode
if(CMAKE_BUILD_TYPE MATCHES Debug)
	include(CTest)
	add_subdirectory(test/src)
endif()

# documentation targets
#if(CMAKE_BUILD_TYPE MATCHES "Release")
#	add_subdirectory(doc)
#endif()
