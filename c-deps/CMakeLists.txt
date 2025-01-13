cmake_minimum_required(VERSION 3.25)
project(beholder
	VERSION 0.2.0
	DESCRIPTION "beholder C++ library"
	LANGUAGES CXX
)
include(GNUInstallDirs)
include(FetchContent)
include(CTest)

# set up dependencies
if (DEFINED bh_third_party_pkg AND DEFINED bh_third_party_md5)
	message(STATUS "Using local archive for library dependencies.")
	list(APPEND CMAKE_MESSAGE_INDENT "    ")
	message(STATUS "archive: ${bh_third_party_pkg}")
	message(STATUS "MD5: ${bh_third_party_md5}")
	list(POP_BACK CMAKE_MESSAGE_INDENT)

	FetchContent_Declare(
		bh-third-party
		URL "file://${bh_third_party_pkg}"
		URL_MD5 "${bh_third_party_md5}"
	)
	FetchContent_MakeAvailable(bh-third-party)
	list(APPEND CMAKE_PREFIX_PATH "${bh-third-party_SOURCE_DIR}/lib/cmake")
	# because deps are installed locally and not system-wide
	include_directories("${bh-third-party_SOURCE_DIR}/include")
else()
	message(STATUS "Using system libraries for dependencies.")
endif()

add_subdirectory(src)
add_subdirectory(test)

file(CREATE_LINK
	"${CMAKE_BINARY_DIR}/compile_commands.json"
	"${CMAKE_BINARY_DIR}/../compile_commands.json"
	SYMBOLIC
)
include(cmake/Packaging.cmake)
