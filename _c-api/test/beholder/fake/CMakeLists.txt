add_library(fake SHARED)
add_library(beholder::fake ALIAS fake)

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
set_target_properties(fake PROPERTIES
	BUILD_WITH_INSTALL_RPATH TRUE
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

#include(CMakePackageConfigHelpers)
#configure_package_config_file(
#	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in"
#	"${CMAKE_CURRENT_BINARY_DIR}/fakeConfig.cmake"
#	INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/fake"
#)
#install(
#	FILES
#		"${CMAKE_CURRENT_BINARY_DIR}/fakeConfig.cmake"
#	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/fake"
#)
