# TODO: add build dependency on shim archive
add_subdirectory(shim)
find_package(pylon REQUIRED)
# archiving, and then unarchiving at target location, is the easiest way
# of moving both files and symlinks from the libdir to our local binary dir
set(pylon_archive "pylon_archive.tar")
set(pylon_archive_files "")
# disgusting hack to generically get the install path
get_target_property(pylon_imported_loc pylon::PylonBase LOCATION)
cmake_path(GET pylon_imported_loc PARENT_PATH pylon_imported_loc)
file(GLOB pylon_archive_files
	"${pylon_imported_loc}/libpylonbase.*"
	"${pylon_imported_loc}/libpylonc.*"
	"${pylon_imported_loc}/libpylonutility.*"
	"${pylon_imported_loc}/libFirmwareUpdate_gcc_v3_1_Basler_pylon.*"
	"${pylon_imported_loc}/libGCBase_gcc_v3_1_Basler_pylon.*"
	"${pylon_imported_loc}/libGenApi_gcc_v3_1_Basler_pylon.*"
	"${pylon_imported_loc}/libLog_gcc_v3_1_Basler_pylon.*"
	"${pylon_imported_loc}/liblog4cpp_gcc_v3_1_Basler_pylon.*"
	"${pylon_imported_loc}/libMathParser_gcc_v3_1_Basler_pylon.*"
	"${pylon_imported_loc}/libNodeMapData_gcc_v3_1_Basler_pylon.*"
	"${pylon_imported_loc}/libXmlParser_gcc_v3_1_Basler_pylon.*"
	"${pylon_imported_loc}/libgxapi.*"
	"${pylon_imported_loc}/libuxapi.*"
	"${pylon_imported_loc}/libpylon_TL_gige.*"
	"${pylon_imported_loc}/libpylon_TL_camemu.*"
)
list(TRANSFORM pylon_archive_files REPLACE "${pylon_imported_loc}/" "")
# build archive to be embedded
set(pylonshim_archive "pylonshim.tar.gz")
set(pylonshim_archive_files "")

#file(GLOB pylonshim_archive_files
#	"${CMAKE_CURRENT_BINARY_DIR}/shim/libpylonshim.*"
#	"${CMAKE_CURRENT_BINARY_DIR}/shim/$<TARGET_PROPERTY:beholder::pylonshim, LIBRARY_OUTPUT_NAME>"
#)
set(libpylonshim "${CMAKE_SHARED_LIBRARY_PREFIX}pylonshim${CMAKE_SHARED_LIBRARY_SUFFIX}")
set(pylonshim_archive_files
	"${CMAKE_CURRENT_BINARY_DIR}/shim/${libpylonshim}"
	"${CMAKE_CURRENT_BINARY_DIR}/shim/${libpylonshim}.0.0.1"
)
list(TRANSFORM pylonshim_archive_files REPLACE "${CMAKE_CURRENT_BINARY_DIR}/shim/" "")
list(APPEND pylonshim_archive_files ${pylon_archive_files})

add_custom_command(
	OUTPUT ${pylonshim_archive}
	DEPENDS beholder::pylonshim
	COMMAND
		${CMAKE_COMMAND} -E
			chdir ${pylon_imported_loc}
			tar cvf ${CMAKE_CURRENT_BINARY_DIR}/shim/${pylon_archive} ${pylon_archive_files}
	COMMAND
		${CMAKE_COMMAND} -E
			chdir ${CMAKE_CURRENT_BINARY_DIR}/shim
			tar xvf ${CMAKE_CURRENT_BINARY_DIR}/shim/${pylon_archive}
	COMMAND
		${CMAKE_COMMAND} -E
			chdir ${CMAKE_CURRENT_BINARY_DIR}/shim
			tar cvzf ${CMAKE_CURRENT_BINARY_DIR}/shim/${pylonshim_archive} ${pylonshim_archive_files}
)
add_custom_target(pylon_camera_shim DEPENDS ${pylonshim_archive})

# NOTE: probably need to use this to speed up the build, because the archive
# either fully rebuilds, or doesn't detect changes in the shim sources/headers.
#
#add_library(pylon_camera OBJECT)
#add_dependencies(pylon_camera pylon_camera_shim)
#target_sources(pylon_camera
#	PRIVATE
#		Camera.cpp
#	PUBLIC
#		FILE_SET HEADERS
#		BASE_DIRS
#			"${bh_base_dir}"
#		FILES
#			Camera.h
#)
#target_link_libraries(pylon_camera PRIVATE beholder::incbin)
##set_source_files_properties(Camera.cpp
##	PROPERTIES
##		OBJECT_DEPENDS ${pylonshim_archive}
##)
#target_compile_definitions(pylon_camera
#	PRIVATE
#		BH_PYLON_EMBED_DIR="${CMAKE_CURRENT_BINARY_DIR}/shim"
#)
#target_sources(beholder PRIVATE $<TARGET_OBJECTS:pylon_camera>)

add_dependencies(beholder pylon_camera_shim)
target_sources(beholder
	PRIVATE
		Camera.cpp
	PUBLIC
		FILE_SET HEADERS
		FILES
			Camera.h
)
# XXX: these might need to go before the sources?
target_compile_definitions(beholder
	PRIVATE
		BH_PYLON_EMBED_DIR="${CMAKE_CURRENT_BINARY_DIR}/shim"
)
