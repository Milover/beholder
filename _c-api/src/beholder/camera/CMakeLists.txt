# TODO: this should be a component library

# the camera support library is always dymanic, because of pylon
add_library(beholder_camera SHARED)
add_library(beholder::beholder_camera ALIAS beholder_camera)

set_target_properties(beholder_camera
	PROPERTIES
		VERSION "${PROJECT_VERSION}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}"
		SOVERSION ${PROJECT_VERSION_MAJOR}
)

#if (CMAKE_BUILD_TYPE OR CMAKE_CONFIGURATION_TYPES)
#	set_target_properties(beholder_camera PROPERTIES ARCHIVE_OUTPUT_NAME "beholder_camera.$<LOWER_CASE:$<CONFIG>>")
#	set_target_properties(beholder_camera PROPERTIES LIBRARY_OUTPUT_NAME "beholder_camera.$<LOWER_CASE:$<CONFIG>>")
#endif()

find_package(pylon REQUIRED)

add_subdirectory(internal)

target_sources(beholder_camera
	PRIVATE
		Camera.cpp
		ParamEntry.cpp
		PylonAPI.cpp
		TransportLayer.cpp
	PUBLIC
		FILE_SET HEADERS
		BASE_DIRS
			"${bh_base_dir}"
		FILES
			BeholderCamera.h
			Camera.h
			Exception.h
			ParamEntry.h
			PylonAPI.h
			TransportLayer.h
)
target_link_libraries(beholder_camera
	PRIVATE
		pylon::pylon
		beholder::beholder
)
target_compile_features(beholder_camera PUBLIC cxx_std_17)

## generate main library global header
#set(INCLUDE_BEHOLDER_HDR "")
#foreach(HEADER ${BEHOLDER_HDR})
#	get_filename_component(HDR ${HEADER} NAME)
#	set(INCLUDE_BEHOLDER_HDR "${INCLUDE_BEHOLDER_HDR}#include \"${HDR}\"\n")
#endforeach()
#configure_file(
#	${CMAKE_SOURCE_DIR}/src/BeholderCamera.h.in
#	${CMAKE_BINARY_DIR}/BeholderCamera.h
#	@ONLY
#)
#list(APPEND BEHOLDER_HDR "${CMAKE_BINARY_DIR}/BeholderCamera.h")

install(
	TARGETS
		beholder_camera
	EXPORT beholder_cameraTargets
	ARCHIVE
		COMPONENT libbeholder_camera-dev
	FILE_SET HEADERS
		COMPONENT libbeholder_camera-dev
	LIBRARY
		COMPONENT libbeholder_camera
		NAMELINK_COMPONENT libbeholder_camera-dev
)
install(
	EXPORT beholder_cameraTargets
	FILE beholder_cameraTargets.cmake
	NAMESPACE beholder::
	COMPONENT libbeholder_camera-dev
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/beholder_camera"
)
export(
	EXPORT beholder_cameraTargets
	FILE "${CMAKE_CURRENT_BINARY_DIR}/beholder_cameraTargets.cmake"
	NAMESPACE beholder::
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/beholder_cameraConfig.cmake"
	INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/beholder_camera"
)
write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/beholder_cameraConfigVersions.cmake"
	VERSION "${PROJECT_VERSION}"
	COMPATIBILITY AnyNewerVersion
)
install(
	FILES
		"${CMAKE_CURRENT_BINARY_DIR}/beholder_cameraConfig.cmake"
		"${CMAKE_CURRENT_BINARY_DIR}/beholder_cameraConfigVersions.cmake"
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/beholder_camera"
)

# generate pkg-config
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/beholder_camera.pc.in"
	"${CMAKE_CURRENT_BINARY_DIR}/cmake/beholder_camera.pc"
	@ONLY
)
install(
	FILES
		"${CMAKE_CURRENT_BINARY_DIR}/cmake/beholder_camera.pc"
	COMPONENT libbeholder_camera-dev
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
)
