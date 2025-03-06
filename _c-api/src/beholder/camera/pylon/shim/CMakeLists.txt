# TODO: this should be a component library

# the camera support library is always dymanic, because of pylon
add_library(pylonshim SHARED)
add_library(beholder::pylonshim ALIAS pylonshim)

set_target_properties(pylonshim
	PROPERTIES
		SOVERSION ${PROJECT_VERSION}
)

#if (CMAKE_BUILD_TYPE OR CMAKE_CONFIGURATION_TYPES)
#	set_target_properties(pylonshim PROPERTIES ARCHIVE_OUTPUT_NAME "pylonshim.$<LOWER_CASE:$<CONFIG>>")
#	set_target_properties(pylonshim PROPERTIES LIBRARY_OUTPUT_NAME "pylonshim.$<LOWER_CASE:$<CONFIG>>")
#endif()

find_package(pylon REQUIRED)

target_sources(pylonshim
	PRIVATE
		API.cpp
		Camera.cpp
		Configurator.cpp
		TransportLayer.cpp
	PRIVATE
		FILE_SET internal
		TYPE HEADERS
		BASE_DIRS
			"${bh_base_dir}"
		FILES
			Configurator.h
			Convert.h
	PUBLIC
		FILE_SET HEADERS
		BASE_DIRS
			"${bh_base_dir}"
		FILES
			API.h
			Camera.h
			Export.h
			TransportLayer.h
)
target_link_libraries(pylonshim PRIVATE pylon::pylon)
target_compile_features(pylonshim PUBLIC cxx_std_17)

install(
	TARGETS
		pylonshim
	EXPORT pylonshimTargets
	ARCHIVE
		COMPONENT dev
	FILE_SET HEADERS
		COMPONENT dev
	LIBRARY
		COMPONENT libpylonshim
		NAMELINK_COMPONENT dev
)
install(
	EXPORT pylonshimTargets
	FILE pylonshimTargets.cmake
	NAMESPACE beholder::
	COMPONENT dev
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/pylonshim"
)
export(
	EXPORT pylonshimTargets
	FILE "${CMAKE_CURRENT_BINARY_DIR}/beholder_cameraTargets.cmake"
	NAMESPACE beholder::
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/pylonshimConfig.cmake"
	INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/pylonshim"
)
write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/pylonshimConfigVersions.cmake"
	VERSION "${PROJECT_VERSION}"
	COMPATIBILITY AnyNewerVersion
)
install(
	FILES
		"${CMAKE_CURRENT_BINARY_DIR}/pylonshimConfig.cmake"
		"${CMAKE_CURRENT_BINARY_DIR}/pylonshimConfigVersions.cmake"
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/pylonshim"
)
