cmake_minimum_required(VERSION 3.25.1)
project(beholder-incbin-internal)

include(GNUInstallDirs)
include(FetchContent)

FetchContent_Declare(
	bh-incbin-internal
	GIT_REPOSITORY https://github.com/graphitemaster/incbin.git
	GIT_TAG 6e576cae5ab5810f25e2631f2e0b80cbe7dc8cbf # HEAD @ 2025-02-02
	GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(bh-incbin-internal)

add_library(incbin INTERFACE)
add_library(beholder::incbin ALIAS incbin)

set_target_properties(incbin
		PROPERTIES
			PUBLIC_HEADER "${bh-incbin-internal_SOURCE_DIR}/incbin.h"
)
install(
	TARGETS incbin
	EXPORT incbinTargets
	PUBLIC_HEADER
	DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/incbin"
	INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)
install(
	EXPORT incbinTargets
	FILE incbinTargets.cmake
	NAMESPACE beholder::
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/incbin"
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/incbinConfig.cmake"
	INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/incbin"
)
install(
	FILES
		"${CMAKE_CURRENT_BINARY_DIR}/incbinConfig.cmake"
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/incbin"
)
