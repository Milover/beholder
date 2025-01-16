add_executable(pylon.test)

find_package(pylon REQUIRED)

target_sources(pylon.test PRIVATE
	pylon.test.cpp
)
target_link_libraries(pylon.test PRIVATE
	pylon::pylon
	GTest::gtest_main
)
target_compile_features(pylon.test PRIVATE cxx_std_20)

include(GoogleTest)
gtest_discover_tests(pylon.test)
