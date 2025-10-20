add_executable(incbin.test)

find_package(incbin REQUIRED)

# we need absolute paths for the binaries
target_compile_definitions(incbin.test
	PRIVATE
		SRC_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
target_sources(incbin.test PRIVATE
	incbin.test.cpp
	incbin_defs.cpp
)
target_link_libraries(incbin.test PRIVATE
	beholder::incbin
	GTest::gtest_main
)
target_compile_features(incbin.test PRIVATE cxx_std_20)

include(GoogleTest)
gtest_discover_tests(incbin.test)
