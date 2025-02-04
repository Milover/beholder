add_executable(zlib.test)

find_package(zlib REQUIRED)

# we need absolute paths for the binaries
target_sources(zlib.test PRIVATE
	zlib.test.cpp
)
target_link_libraries(zlib.test PRIVATE
	ZLIB::ZLIBSTATIC
	GTest::gtest_main
)
target_compile_features(zlib.test PRIVATE cxx_std_20)

include(GoogleTest)
gtest_discover_tests(zlib.test)
