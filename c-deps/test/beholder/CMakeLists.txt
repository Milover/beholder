add_executable(beholder.test)
target_sources(beholder.test
	PRIVATE
		FILE_SET HEADERS
		BASE_DIRS
			"${CMAKE_CURRENT_SOURCE_DIR}"
		FILES
			Testing.h
	PRIVATE
		image.test.cpp
		neural.test.cpp
		Testing.cpp
)
target_link_libraries(beholder.test
	PRIVATE
		beholder::beholder
		GTest::gtest_main
)
target_compile_features(beholder.test PRIVATE cxx_std_20)

add_executable(beholder_camera.test)
target_sources(beholder_camera.test
	PRIVATE
		FILE_SET HEADERS
		BASE_DIRS
			"${CMAKE_CURRENT_SOURCE_DIR}"
		FILES
			Testing.h
	PRIVATE
		camera.test.cpp
		Testing.cpp
)
target_link_libraries(beholder_camera.test
	PRIVATE
		beholder::beholder_camera
		beholder::beholder
		GTest::gtest_main
)
target_compile_features(beholder_camera.test PRIVATE cxx_std_20)

include(GoogleTest)
gtest_discover_tests(beholder.test)
gtest_discover_tests(beholder_camera.test)
