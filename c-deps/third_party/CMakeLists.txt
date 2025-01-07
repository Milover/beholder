cmake_minimum_required(VERSION 3.25.1)

# FIXME: dependency versions really should be CLI args, thing will get
# hard to manage otherwise.

include(cmake/bhGrabCli.cmake)
bh_grab_cli(bh_cmake_args)

project(beholder-thirdparty
	VERSION 0.1.0
	DESCRIPTION "beholder C++ library third party dependencies"
	LANGUAGES CXX
)
include(GNUInstallDirs)

set(bh_staging_prefix "${CMAKE_BINARY_DIR}/staging")

include(ExternalProject)
include(FetchContent)
add_subdirectory(src)

# library setup
add_library(thirdparty INTERFACE)
add_library(beholder::thirdparty ALIAS thirdparty)
set_property(TARGET thirdparty
	PROPERTY SOVERSION "${CMAKE_PROJECT_VERSION}"
)
add_dependencies(thirdparty bh-opencv bh-leptonica bh-tesseract bh-pylon)

# NOTE: not sure if this necessarily has to go last/after installs
ExternalProject_Add(
	bh-thirdparty-test
	URL "${CMAKE_CURRENT_SOURCE_DIR}/test"
	INSTALL_DIR "${bh_staging_prefix}"
	TEST_AFTER_INSTALL TRUE
	INSTALL_COMMAND ""
	DEPENDS thirdparty
	CMAKE_ARGS ${bh_cmake_args}
		-Dbh_binary_dir=${CMAKE_CURRENT_BINARY_DIR}
		-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
		-DCMAKE_PREFIX_PATH=<INSTALL_DIR>
)

install(
	TARGETS
		thirdparty
	EXPORT beholder-thirdpartyTargets
	DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)
install(
	EXPORT beholder-thirdpartyTargets
	FILE beholder-thirdpartyTargets.cmake
	NAMESPACE beholder::
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/beholder-thirdparty"
)
#export(
#	EXPORT beholder-thirdpartyTargets
#	NAMESPACE beholder::
#	FILE "${CMAKE_CURRENT_BINARY_DIR}/beholder-thirdpartyTargets.cmake"
#)
include(CMakePackageConfigHelpers)
configure_package_config_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/beholder-thirdpartyConfig.cmake"
	INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/beholder-thirdparty"
)
#write_basic_package_version_file(
#	"${CMAKE_CURRENT_BINARY_DIR}/beholder-thirdpartyConfigVersion.cmake"
#	VERSION "${CMAKE_PROJECT_VERSION}"
#	COMPATIBILITY AnyNewerVersion
#)
install(
	FILES
		"${CMAKE_CURRENT_BINARY_DIR}/beholder-thirdpartyConfig.cmake"
		#"${CMAKE_CURRENT_BINARY_DIR}/beholder-thirdpartyConfigVersion.cmake"
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/beholder-thirdparty"
)
#install(DIRECTORY "${bh_staging_prefix}/bin/" TYPE BIN)
install(DIRECTORY "${bh_staging_prefix}/lib/" TYPE LIB)
install(DIRECTORY "${bh_staging_prefix}/include/" TYPE INCLUDE)
install(DIRECTORY "${bh_staging_prefix}/share/" TYPE DATA)

include(cmake/Packaging.cmake)
#include(cmake/Docker.cmake)
