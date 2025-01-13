cmake_minimum_required(VERSION 3.25.1)

# FIXME: dependency versions really should be CLI args, thing will get
# hard to manage otherwise.

include(cmake/bhGrabCli.cmake)
bh_grab_cli(bh_cmake_args)

project(beholder-third-party
	VERSION 0.3.0
	DESCRIPTION "beholder C++ library third party dependencies"
	LANGUAGES CXX
)
include(GNUInstallDirs)

set(bh_staging_prefix "${CMAKE_BINARY_DIR}/staging")

include(ExternalProject)
include(FetchContent)
add_subdirectory(src)

# library setup
add_library(third_party INTERFACE)
add_library(beholder::third_party ALIAS third_party)
set_property(TARGET third_party
	PROPERTY SOVERSION "${CMAKE_PROJECT_VERSION}"
)
add_dependencies(third_party bh-opencv bh-leptonica bh-tesseract bh-pylon)

# NOTE: not sure if this necessarily has to go last/after installs
ExternalProject_Add(
	bh-third-party-test
	URL "${CMAKE_CURRENT_SOURCE_DIR}/test"
	INSTALL_DIR "${bh_staging_prefix}"
	TEST_AFTER_INSTALL TRUE
	INSTALL_COMMAND ""
	DEPENDS third_party
	CMAKE_ARGS ${bh_cmake_args}
		-Dbh_binary_dir=${CMAKE_CURRENT_BINARY_DIR}
		-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
		-DCMAKE_PREFIX_PATH=<INSTALL_DIR>
)

install(
	TARGETS
		third_party
	EXPORT beholder_third_partyTargets
	DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)
install(
	EXPORT beholder_third_partyTargets
	FILE beholder_third_partyTargets.cmake
	NAMESPACE beholder::
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/beholder_third_party"
)
#export(
#	EXPORT beholder_third_partyTargets
#	NAMESPACE beholder::
#	FILE "${CMAKE_CURRENT_BINARY_DIR}/beholder_third_partyTargets.cmake"
#)
include(CMakePackageConfigHelpers)
configure_package_config_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/beholder_third_partyConfig.cmake"
	INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/beholder_third_party"
)
#write_basic_package_version_file(
#	"${CMAKE_CURRENT_BINARY_DIR}/beholder_third_partyConfigVersion.cmake"
#	VERSION "${CMAKE_PROJECT_VERSION}"
#	COMPATIBILITY AnyNewerVersion
#)
install(
	FILES
		"${CMAKE_CURRENT_BINARY_DIR}/beholder_third_partyConfig.cmake"
		#"${CMAKE_CURRENT_BINARY_DIR}/beholder_third_partyConfigVersion.cmake"
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/beholder_third_party"
)
#install(DIRECTORY "${bh_staging_prefix}/bin/" TYPE BIN)
install(DIRECTORY "${bh_staging_prefix}/lib/" TYPE LIB)
install(DIRECTORY "${bh_staging_prefix}/include/" TYPE INCLUDE)
install(DIRECTORY "${bh_staging_prefix}/share/" TYPE DATA)

include(cmake/Packaging.cmake)
#include(cmake/Docker.cmake)
