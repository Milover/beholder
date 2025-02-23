cmake_minimum_required(VERSION 3.25)
project(fake
	VERSION 0.0.1
	DESCRIPTION "a fake library, for fake people"
	LANGUAGES CXX
)
include(GNUInstallDirs)

add_library(fake SHARED)
add_library(beholder::fake ALIAS fake)

set_target_properties(fake
	PROPERTIES
		SOVERSION ${PROJECT_VERSION}
		BUILD_WITH_INSTALL_RPATH TRUE
)
target_sources(fake
	PRIVATE
		Fake.cpp
	PUBLIC
		FILE_SET HEADERS
		BASE_DIRS
			"${CMAKE_CURRENT_SOURCE_DIR}/../"
		FILES
			Fake.h
			FakeExport.h
)

install(
	TARGETS
		fake
	EXPORT fakeTargets
	ARCHIVE
		COMPONENT libfake-dev
	FILE_SET HEADERS
		COMPONENT libfake-dev
	LIBRARY
		COMPONENT libfake
		NAMELINK_COMPONENT libfake-dev
)
install(
	EXPORT fakeTargets
	FILE fakeTargets.cmake
	NAMESPACE beholder::
	COMPONENT libfake-dev
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/fake"
)
export(
	EXPORT fakeTargets
	FILE "${CMAKE_CURRENT_BINARY_DIR}/fakeTargets.cmake"
	NAMESPACE beholder::
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/fakeConfig.cmake"
	INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/fake"
)
write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/fakeConfigVersions.cmake"
	VERSION "${PROJECT_VERSION}"
	COMPATIBILITY AnyNewerVersion
)
install(
	FILES
		"${CMAKE_CURRENT_BINARY_DIR}/fakeConfig.cmake"
		"${CMAKE_CURRENT_BINARY_DIR}/fakeConfigVersions.cmake"
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/fake"
)
