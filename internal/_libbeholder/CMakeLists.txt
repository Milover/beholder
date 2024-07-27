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

# define dependency vars
set(BEHOLDER_INCLUDE_DIRS "")
set(BEHOLDER_LIBRARIES "")

# FetchContent
#include(FetchContent)

# Leptonica
find_package(Leptonica REQUIRED)
list(APPEND BEHOLDER_INCLUDE_DIRS ${Leptonica_INCLUDE_DIRS})
list(APPEND BEHOLDER_LIBRARIES ${Leptonica_LIBRARIES})

# Tesseract
find_package(Tesseract REQUIRED)
list(APPEND BEHOLDER_INCLUDE_DIRS ${Tesseract_INCLUDE_DIRS})
list(APPEND BEHOLDER_LIBRARIES "Tesseract::libtesseract")

# OpenCV
find_package(OpenCV REQUIRED)
list(APPEND BEHOLDER_INCLUDE_DIRS ${OpenCV_INCLUDE_DIRS})
list(APPEND BEHOLDER_LIBRARIES ${OpenCV_LIBS})

# pylon
find_package(pylon 7.5.0 REQUIRED)
list(APPEND BEHOLDER_INCLUDE_DIRS ${pylon_INCLUDE_DIRS})
list(APPEND BEHOLDER_LIBRARIES "pylon::pylon")

# Threads
#find_package(Threads REQUIRED)
#list(APPEND BEHOLDER_LIBRARIES "Threads::Threads")

# random linker stuff
#list(APPEND BEHOLDER_LIBRARIES "stdc++fs")

# add sources and headers
file(
	GLOB
	BEHOLDER_SRC
	CONFIGURE_DEPENDS
	"${PROJECT_SOURCE_DIR}/src/*.cpp"
	"${PROJECT_SOURCE_DIR}/src/camera/*.cpp"
	"${PROJECT_SOURCE_DIR}/src/image/*.cpp"
	"${PROJECT_SOURCE_DIR}/src/image/ops/*.cpp"
	"${PROJECT_SOURCE_DIR}/src/ocr/*.cpp"
)

file(
	GLOB
	BEHOLDER_HDR
	CONFIGURE_DEPENDS
	"${PROJECT_SOURCE_DIR}/src/*.h"
	"${PROJECT_SOURCE_DIR}/src/camera/*.h"
	"${PROJECT_SOURCE_DIR}/src/image/*.h"
	"${PROJECT_SOURCE_DIR}/src/image/ops/*.h"
	"${PROJECT_SOURCE_DIR}/src/ocr/*.h"
)

list(
	APPEND
	BEHOLDER_INCLUDE_DIRS 
	"${PROJECT_SOURCE_DIR}/src"
	"${PROJECT_SOURCE_DIR}/src/camera"
	"${PROJECT_SOURCE_DIR}/src/image"
	"${PROJECT_SOURCE_DIR}/src/image/ops"
	"${PROJECT_SOURCE_DIR}/src/ocr"
)

# main library target
add_library (beholder SHARED)
set_target_properties(beholder PROPERTIES
	#	BUILD_WITH_INSTALL_RPATH FALSE
	#	INSTALL_RPATH_USE_LINK_PATH TRUE
	#	INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib:${CMAKE_INSTALL_PREFIX}/share/lib"
	INTERPROCEDURAL_OPTIMIZATION TRUE
)
target_include_directories(beholder PUBLIC ${BEHOLDER_INCLUDE_DIRS})
target_link_libraries(beholder PUBLIC ${BEHOLDER_LIBRARIES})
#target_link_options(beholder
#	PRIVATE
#	-Wl,--whole-archive
#	${BEHOLDER_LIBRARIES}
#	-Wl,--no-whole-archive
#	)
target_sources(beholder PRIVATE ${BEHOLDER_SRC})
target_sources(beholder PUBLIC FILE_SET HEADERS FILES ${BEHOLDER_HDR})

# generate pkg-config
configure_file(
	${CMAKE_SOURCE_DIR}/beholder.pc.in
	${CMAKE_BINARY_DIR}/beholder.pc
	@ONLY
)

# install targets
install(TARGETS beholder LIBRARY DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")
install(FILES ${BEHOLDER_HDR} DESTINATION "${CMAKE_INSTALL_PREFIX}/include/beholder")

# install the pkg-config
install(FILES "${CMAKE_BINARY_DIR}/beholder.pc"
	DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig"
)

# enable testing in debug mode
if(CMAKE_BUILD_TYPE MATCHES Debug)
	include(CTest)
	add_subdirectory(test/src)
endif()

# documentation targets
#if(CMAKE_BUILD_TYPE MATCHES "Release")
#	add_subdirectory(doc)
#endif()
