cmake_minimum_required(VERSION 3.25)
project(beholder
	VERSION 0.4.0
	DESCRIPTION "beholder C++ library"
	LANGUAGES CXX
)
include(GNUInstallDirs)
include(FetchContent)
include(CTest)

# set up dependencies
#
# NOTE: the third-party dependencies can also manually be unpacked/installed
# into the staging directory, provided CMAKE_INSTALL_PREFIX is properly set,
# CMake should correctly find/include/link all dependencies.
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
	list(APPEND CMAKE_PREFIX_PATH "${bh-third-party_SOURCE_DIR}/${CMAKE_INSTALL_LIBDIR}/cmake")
else()
	message(STATUS "Using system libraries for dependencies.")
endif()

message(CHECK_START "Finding linters")
list(APPEND CMAKE_MESSAGE_INDENT "    ")
unset(bh_missing_linters)

message(CHECK_START "Finding clang-tidy")
find_program(found_clang_tidy clang-tidy)
if (found_clang_tidy)
	message(CHECK_PASS "found")
	set(CMAKE_CXX_CLANG_TIDY clang-tidy)
	# could export fixes here
else()
	message(CHECK_FAIL "not found")
endif()

list(POP_BACK CMAKE_MESSAGE_INDENT)
if (bh_missing_linters)
	message(CHECK_FAIL "linters missing: ${bh_missing_linters}")
else()
	message(CHECK_PASS "all linters found")
endif()

add_subdirectory(src)
add_subdirectory(test)

file(CREATE_LINK
	"${CMAKE_BINARY_DIR}/compile_commands.json"
	"${CMAKE_BINARY_DIR}/../compile_commands.json"
	SYMBOLIC
)
include(cmake/Packaging.cmake)
