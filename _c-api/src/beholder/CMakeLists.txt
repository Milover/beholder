add_library(beholder)
add_library(beholder::beholder ALIAS beholder)

set_target_properties(beholder
	PROPERTIES
		VERSION "${PROJECT_VERSION}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}"
		SOVERSION ${PROJECT_VERSION_MAJOR}
		COMPILE_OPTIONS $<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:-DBEHOLDER_STATIC_DEFINE>
)

#if (CMAKE_BUILD_TYPE OR CMAKE_CONFIGURATION_TYPES)
#	set_target_properties(beholder PROPERTIES ARCHIVE_OUTPUT_NAME "beholder.$<LOWER_CASE:$<CONFIG>>")
#	set_target_properties(beholder PROPERTIES LIBRARY_OUTPUT_NAME "beholder.$<LOWER_CASE:$<CONFIG>>")
#endif()

find_package(Iconv REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Tesseract REQUIRED)

# convenience variable, so that we can propagate it to sub-directories easily
cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH bh_base_dir)

target_sources(beholder
	PUBLIC
		FILE_SET HEADERS
		BASE_DIRS
			"${bh_base_dir}"
		FILES
			Beholder.h
			BeholderExport.h
)
# Tesseract's include dirs are necessary when Tesseract is installed to a
# non-default location
target_include_directories(beholder
	PRIVATE
		${OpenCV_INCLUDE_DIRS}
		${Tesseract_INCLUDE_DIRS}
)
target_link_libraries(beholder
	PRIVATE
		Tesseract::libtesseract
		${OpenCV_LIBS}
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
target_compile_features(beholder PUBLIC cxx_std_17)

add_subdirectory(util)
add_subdirectory(capi)
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
