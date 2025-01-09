add_executable(beholder.test)
target_sources(beholder.test
	PRIVATE
		image.test.cpp
		neural.test.cpp
)
target_link_libraries(beholder.test
	PRIVATE
		beholder::beholder
		GTest::gtest_main
)
target_compile_features(beholder.test PRIVATE cxx_std_17)

include(GoogleTest)
gtest_discover_tests(beholder.test)
