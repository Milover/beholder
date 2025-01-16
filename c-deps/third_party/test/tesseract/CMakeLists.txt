add_executable(tesseract.test)

find_package(Leptonica REQUIRED)
find_package(Tesseract REQUIRED)

target_sources(tesseract.test PRIVATE
	tesseract.test.cpp
)
target_include_directories(tesseract.test PRIVATE
	${Leptonica_INCLUDE_DIRS}
)
target_link_directories(tesseract.test PRIVATE
	${CMAKE_INSTALL_FULL_LIBDIR}
)
target_link_libraries(tesseract.test PRIVATE
	Tesseract::libtesseract
	GTest::gtest_main
)
target_compile_features(tesseract.test PRIVATE cxx_std_20)

include(GoogleTest)
gtest_discover_tests(tesseract.test)
