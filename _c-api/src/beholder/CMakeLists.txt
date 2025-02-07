add_library(beholder)
add_library(beholder::beholder ALIAS beholder)

set_target_properties(beholder
	PROPERTIES
		SOVERSION ${PROJECT_VERSION}
		COMPILE_OPTIONS $<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:-DBEHOLDER_STATIC_DEFINE>
)

#if (CMAKE_BUILD_TYPE OR CMAKE_CONFIGURATION_TYPES)
#	set_target_properties(beholder PROPERTIES ARCHIVE_OUTPUT_NAME "beholder.$<LOWER_CASE:$<CONFIG>>")
#	set_target_properties(beholder PROPERTIES LIBRARY_OUTPUT_NAME "beholder.$<LOWER_CASE:$<CONFIG>>")
#endif()

# we only use these OpenCV modules
list(APPEND bh_ocv_modules
	core
	imgproc
	imgcodecs
	videoio
	video
	dnn
	photo
	gapi
)

find_package(incbin REQUIRED)
find_package(zlib REQUIRED)
find_package(OpenCV REQUIRED ${bh_ocv_modules})
find_package(Tesseract REQUIRED)
find_package(pylon REQUIRED)

# TODO: clean this up
# disgusting hack to get the generic install path
get_target_property(pylon_imported_loc pylon::PylonBase LOCATION)
cmake_path(GET pylon_imported_loc PARENT_PATH pylon_imported_loc)
file(GLOB pylon_lib_files
	"${pylon_imported_loc}/libpylonbase.so*"
	"${pylon_imported_loc}/libpylonc.so*"
	"${pylon_imported_loc}/libpylonutility.so*"
	"${pylon_imported_loc}/libFirmwareUpdate_gcc_v3_1_Basler_pylon.so"
	"${pylon_imported_loc}/libGCBase_gcc_v3_1_Basler_pylon.so"
	"${pylon_imported_loc}/libGenApi_gcc_v3_1_Basler_pylon.so"
	"${pylon_imported_loc}/libLog_gcc_v3_1_Basler_pylon.so"
	"${pylon_imported_loc}/liblog4cpp_gcc_v3_1_Basler_pylon.so"
	"${pylon_imported_loc}/libMathParser_gcc_v3_1_Basler_pylon.so"
	"${pylon_imported_loc}/libNodeMapData_gcc_v3_1_Basler_pylon.so"
	"${pylon_imported_loc}/libXmlParser_gcc_v3_1_Basler_pylon.so"
	"${pylon_imported_loc}/libgxapi.so*"
	"${pylon_imported_loc}/libuxapi.so*"
	"${pylon_imported_loc}/libpylon_TL_gige.so"
	"${pylon_imported_loc}/libpylon_TL_camemu.so"
)
add_custom_command(
	OUTPUT pylon.tar.gz
	COMMAND
		${CMAKE_COMMAND} -E tar cvz pylon.tar.gz ${pylon_lib_files}
)

# convenience variable, so that we can propagate it to sub-directories easily
cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH bh_base_dir)

target_sources(beholder
	PRIVATE
		pylon.tar.gz
	PUBLIC
		FILE_SET HEADERS
		BASE_DIRS
			"${bh_base_dir}"
		FILES
			Beholder.h
			BeholderExport.h
)
target_compile_definitions(beholder
	PRIVATE
		BH_EMBED_DIR="${CMAKE_CURRENT_BINARY_DIR}"
		ZLIB_CONST
)
# Tesseract's include dirs are necessary when Tesseract is installed to a
# non-default location
target_include_directories(beholder
	PRIVATE
		${OpenCV_INCLUDE_DIRS}
		${Tesseract_INCLUDE_DIRS}
)
list(TRANSFORM bh_ocv_modules PREPEND "opencv_" OUTPUT_VARIABLE bh_ocv_libs)
target_link_libraries(beholder
	PRIVATE
		beholder::incbin
		ZLIB::ZLIBSTATIC
		Tesseract::libtesseract
		#${OpenCV_LIBS}
		${bh_ocv_libs}
)
# This is necessary so that OpenCV's 3rd-party dependencies get
# properly propagated to other components
#
# FIXME: we should handle those dependencies ourselves, so we don't have to
# fuck around with trying to collect all these stupid paths.
# Also no idea what happens if someone else is trying to use us --- would they
# also have to set all these paths or what?
target_link_directories(beholder BEFORE
	PUBLIC
		$<BUILD_INTERFACE:${OpenCV_INSTALL_PATH}/${CMAKE_INSTALL_LIBDIR}>
		$<BUILD_INTERFACE:${OpenCV_INSTALL_PATH}/${CMAKE_INSTALL_LIBDIR}/opencv4/3rdparty>
)
target_compile_features(beholder PUBLIC cxx_std_20)

add_subdirectory(util)
add_subdirectory(capi)
add_subdirectory(embed)
add_subdirectory(image)
add_subdirectory(neural)

add_subdirectory(camera)

## generate main library global header
#set(INCLUDE_BEHOLDER_HDR "")
#foreach(HEADER ${BEHOLDER_HDR})
#	get_filename_component(HDR ${HEADER} NAME)
#	set(INCLUDE_BEHOLDER_HDR "${INCLUDE_BEHOLDER_HDR}#include \"${HDR}\"\n")
#endforeach()
#configure_file(
#	${CMAKE_SOURCE_DIR}/src/libbeholder.h.in
#	${CMAKE_BINARY_DIR}/libbeholder.h
#	@ONLY
#)
#list(APPEND BEHOLDER_HDR "${CMAKE_BINARY_DIR}/libbeholder.h")

install(
	TARGETS
		beholder
	EXPORT beholderTargets
	ARCHIVE
		COMPONENT libbeholder-dev
	FILE_SET HEADERS
		COMPONENT libbeholder-dev
	LIBRARY
		COMPONENT libbeholder
		NAMELINK_COMPONENT libbeholder-dev
)
install(
	EXPORT beholderTargets
	FILE beholderTargets.cmake
	NAMESPACE beholder::
	COMPONENT libbeholder-dev
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/beholder"
)
export(
	EXPORT beholderTargets
	FILE "${CMAKE_CURRENT_BINARY_DIR}/beholderTargets.cmake"
	NAMESPACE beholder::
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/beholderConfig.cmake"
	INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/beholder"
)
write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/beholderConfigVersions.cmake"
	VERSION "${PROJECT_VERSION}"
	COMPATIBILITY AnyNewerVersion
)
install(
	FILES
		"${CMAKE_CURRENT_BINARY_DIR}/beholderConfig.cmake"
		"${CMAKE_CURRENT_BINARY_DIR}/beholderConfigVersions.cmake"
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/beholder"
)

# generate pkg-config
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/beholder.pc.in"
	"${CMAKE_CURRENT_BINARY_DIR}/cmake/beholder.pc"
	@ONLY
)
install(
	FILES
		"${CMAKE_CURRENT_BINARY_DIR}/cmake/beholder.pc"
	COMPONENT libbeholder-dev
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
)
