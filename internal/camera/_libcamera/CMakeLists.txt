#
# camera - Copyright (C) 2024 P. Milovic
#
# See the LICENSE file for license information

# cmake options
cmake_minimum_required(VERSION 3.25)

# project name
project(camera)

# camera support is Linux only
if (NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
	message(FATAL_ERROR "Camera support can only be built on Linux")
endif()

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
set(CAMERA_INCLUDE_DIRS "")
set(CAMERA_LIBRARIES "")

# FetchContent
#include(FetchContent)

# pylon
set(pylon_DIR "${PROJECT_SOURCE_DIR}/share/lib/cmake/pylon")
find_package(pylon 7.5.0 REQUIRED)
list(APPEND CAMERA_INCLUDE_DIRS ${pylon_INCLUDE_DIRS})
list(APPEND CAMERA_LIBRARIES "pylon::pylon")

# Threads
#find_package(Threads REQUIRED)
#list(APPEND CAMERA_LIBRARIES "Threads::Threads")

# random linker stuff
#list(APPEND CAMERA_LIBRARIES "stdc++fs")

# add sources and headers
file(
	GLOB
	CAMERA_SRC
	CONFIGURE_DEPENDS
	"${PROJECT_SOURCE_DIR}/src/*.cpp")

file(
	GLOB
	CAMERA_HDR
	CONFIGURE_DEPENDS
	"${PROJECT_SOURCE_DIR}/src/*.h")

list(
	APPEND
	CAMERA_INCLUDE_DIRS
	"${PROJECT_SOURCE_DIR}/src")

# main library target
add_library (camera SHARED)
set_property(TARGET camera
	PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
target_include_directories(camera PUBLIC ${CAMERA_INCLUDE_DIRS})
target_link_libraries(camera PUBLIC ${CAMERA_LIBRARIES})
#target_link_options(camera
#	PRIVATE
#	-Wl,--whole-archive
#	${CAMERA_LIBRARIES}
#	-Wl,--no-whole-archive
#	)
target_sources(camera PRIVATE ${CAMERA_SRC})
target_sources(camera PUBLIC FILE_SET HEADERS FILES ${CAMERA_HDR})

# install targets
install(TARGETS camera DESTINATION "${PROJECT_SOURCE_DIR}/lib")
install(FILES ${CAMERA_HDR} DESTINATION "${PROJECT_SOURCE_DIR}/include")

# enable testing
include(CTest)

if(CMAKE_BUILD_TYPE MATCHES Debug)
	add_subdirectory(test/src)
endif()

# documentation targets
#if(CMAKE_BUILD_TYPE MATCHES "Release")
#	add_subdirectory(doc)
#endif()
