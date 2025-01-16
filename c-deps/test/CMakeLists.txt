FetchContent_Declare(
	googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG HEAD
	GIT_SHALLOW TRUE
	FIND_PACKAGE_ARGS NAMES GTest
)

set(INSTALL_GTEST FALSE)

FetchContent_MakeAvailable(googletest)

add_subdirectory(beholder)
