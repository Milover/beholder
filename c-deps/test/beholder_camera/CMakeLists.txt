add_executable(beholder_camera.test)
target_sources(beholder_camera.test
	PRIVATE
		camera.test.cpp
)
target_link_libraries(beholder_camera.test
	PRIVATE
		beholder::beholder_camera
		GTest::gtest_main
)
target_compile_features(beholder_camera.test PRIVATE cxx_std_17)

include(GoogleTest)
gtest_discover_tests(beholder_camera.test)
