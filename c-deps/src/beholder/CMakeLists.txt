add_library(beholder)
add_library(beholder::beholder ALIAS beholder)

set_property(TARGET beholder PROPERTY SOVERSION "${PROJECT_VERSION}")

#if (CMAKE_BUILD_TYPE OR CMAKE_CONFIGURATION_TYPES)
#	set_target_properties(beholder PROPERTIES ARCHIVE_OUTPUT_NAME "beholder.$<LOWER_CASE:$<CONFIG>>")
#	set_target_properties(beholder PROPERTIES LIBRARY_OUTPUT_NAME "beholder.$<LOWER_CASE:$<CONFIG>>")
#endif()

find_package(Iconv REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Tesseract REQUIRED)

target_sources(beholder
	PUBLIC
		FILE_SET HEADERS
		BASE_DIRS
			"${CMAKE_CURRENT_SOURCE_DIR}"
		FILES
			#libbeholder.h
)
target_include_directories(beholder
	PRIVATE
	#${Tesseract_INCLUDE_DIRS}	# FIXME: this shouldn't be necessary
		${OpenCV_INCLUDE_DIRS}
)
target_link_libraries(beholder
	PRIVATE
		Tesseract::libtesseract
		${OpenCV_LIBS}
)
target_compile_features(beholder PUBLIC cxx_std_17)

add_subdirectory(util)
add_subdirectory(capi)
add_subdirectory(image)
add_subdirectory(neural)

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
