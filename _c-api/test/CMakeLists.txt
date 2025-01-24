FetchContent_Declare(
	googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG HEAD
	GIT_SHALLOW TRUE
	FIND_PACKAGE_ARGS NAMES GTest
)

set(INSTALL_GTEST FALSE)

FetchContent_MakeAvailable(googletest)

# disable clang-tidy checks for GTest
set_target_properties(gtest PROPERTIES CXX_CLANG_TIDY "")
set_target_properties(gtest_main PROPERTIES CXX_CLANG_TIDY "")
set_target_properties(gmock PROPERTIES CXX_CLANG_TIDY "")
set_target_properties(gmock_main PROPERTIES CXX_CLANG_TIDY "")

add_subdirectory(beholder)
