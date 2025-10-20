cmake_minimum_required(VERSION 3.25)
project(beholder-pylonshim
	VERSION 0.0.1
	LANGUAGES CXX
)
include(GNUInstallDirs)

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
target_compile_features(pylonshim PUBLIC cxx_std_20)

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
	FILE "${CMAKE_CURRENT_BINARY_DIR}/pylonshimTargets.cmake"
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

## archiving, and then unarchiving at target location, is the easiest way
## of moving both files and symlinks from the libdir to our local binary dir
#set(pylon_archive "pylon_archive.tar")
#set(pylon_archive_files "")
## disgusting hack to generically get the install path
#get_target_property(pylon_imported_loc pylon::PylonBase LOCATION)
#cmake_path(GET pylon_imported_loc PARENT_PATH pylon_imported_loc)
#file(GLOB pylon_archive_files
#	"${pylon_imported_loc}/libpylonbase.*"
#	"${pylon_imported_loc}/libpylonc.*"
#	"${pylon_imported_loc}/libpylonutility.*"
#	"${pylon_imported_loc}/libFirmwareUpdate_gcc_v3_1_Basler_pylon.*"
#	"${pylon_imported_loc}/libGCBase_gcc_v3_1_Basler_pylon.*"
#	"${pylon_imported_loc}/libGenApi_gcc_v3_1_Basler_pylon.*"
#	"${pylon_imported_loc}/libLog_gcc_v3_1_Basler_pylon.*"
#	"${pylon_imported_loc}/liblog4cpp_gcc_v3_1_Basler_pylon.*"
#	"${pylon_imported_loc}/libMathParser_gcc_v3_1_Basler_pylon.*"
#	"${pylon_imported_loc}/libNodeMapData_gcc_v3_1_Basler_pylon.*"
#	"${pylon_imported_loc}/libXmlParser_gcc_v3_1_Basler_pylon.*"
#	"${pylon_imported_loc}/libgxapi.*"
#	"${pylon_imported_loc}/libuxapi.*"
#	"${pylon_imported_loc}/libpylon_TL_gige.*"
#	"${pylon_imported_loc}/libpylon_TL_camemu.*"
#)
#list(TRANSFORM pylon_archive_files REPLACE "${pylon_imported_loc}/" "")
#add_custom_command(
#	OUTPUT ${pylon_archive}
#	COMMAND
#		${CMAKE_COMMAND} -E
#			chdir ${pylon_imported_loc}
#			tar cvf ${CMAKE_CURRENT_BINARY_DIR}/${pylon_archive} ${pylon_archive_files}
#)
## build archive to be embedded
#set(pylonshim_archive "pylonshim.tar.gz")
#set(pylonshim_archive_files "")
#file(GLOB pylonshim_archive_files
#	"${CMAKE_CURRENT_BINARY_DIR}/libpylonshim.*"
#)
#list(TRANSFORM pylonshim_archive_files REPLACE "${CMAKE_CURRENT_BINARY_DIR}/" "")
#list(APPEND pylonshim_archive_files ${pylon_archive_files})
#add_custom_command(
#	OUTPUT ${pylonshim_archive}
#	DEPENDS
#		${pylon_archive}
#		pylonshim
#	COMMAND
#		${CMAKE_COMMAND} -E
#			chdir ${CMAKE_CURRENT_BINARY_DIR}
#			tar xvf ${CMAKE_CURRENT_BINARY_DIR}/${pylon_archive}
#			tar cvzf ${CMAKE_CURRENT_BINARY_DIR}/${pylonshim_archive} ${pylonshim_archive_files}
#)
#
#target_sources(beholder
#	PRIVATE ${pylonshim_archive}
#)
