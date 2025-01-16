add_executable(opencv.test)

find_package(Iconv REQUIRED)
find_package(OpenCV REQUIRED)

target_sources(opencv.test PRIVATE
	opencv.test.cpp
)
target_include_directories(opencv.test PRIVATE
	${OpenCV_INCLUDE_DIRS}
)
target_link_libraries(opencv.test PRIVATE
	${OpenCV_LIBS}
	GTest::gtest_main
)
target_compile_features(opencv.test PRIVATE cxx_std_20)

include(GoogleTest)
gtest_discover_tests(opencv.test)
