find_package(incbin REQUIRED)

FetchContent_Declare(
	googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG HEAD
	GIT_SHALLOW TRUE
	FIND_PACKAGE_ARGS NAMES GTest
)
set(INSTALL_GTEST FALSE)
FetchContent_MakeAvailable(googletest)

# set properties for GTest
list(APPEND _bh_gtest_tgts gtest gtest_main gmock gmock_main)
set_target_properties(gtest gtest_main gmock gmock_main
	PROPERTIES
		CXX_CLANG_TIDY ""
		COMPILE_OPTIONS "-Wno-deprecated-declarations"
)
add_subdirectory(beholder)
