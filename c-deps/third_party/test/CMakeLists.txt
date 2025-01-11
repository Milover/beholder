cmake_minimum_required(VERSION 3.25.1)
project(beholder-third-party-test)

include(CTest)
include(FetchContent)

FetchContent_Declare(
	googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG HEAD
	GIT_SHALLOW TRUE
	FIND_PACKAGE_ARGS NAMES GTest
)
FetchContent_MakeAvailable(googletest)

# These are here because all test targets use them, but we can't supply them
# on the CLI or through the environment without also affecting how dependencies
# are compiled, which we don't want.
add_compile_options("-Wall" "-Wextra" "-Wpedantic" "-Wshadow")

add_subdirectory(opencv)
# NOTE: Leptonica is an internal Tesseract dependency, so we're not testing
# it directly. If the Tesseract test passes, we're good to go.
#add_subdirectory(leptonica)
add_subdirectory(tesseract)
add_subdirectory(pylon)

# symlink the latest compile_commands.json to the root of the build directory
# so tooling can easily find it
file(CREATE_LINK
	"${CMAKE_BINARY_DIR}/compile_commands.json"
	"${bh_binary_dir}/../compile_commands.json"
	SYMBOLIC
)
