# the fake library should get built and archived first so that it can get
# embedded into the test binary
add_subdirectory(fake)
set(libfake "${CMAKE_SHARED_LIBRARY_PREFIX}fake${CMAKE_SHARED_LIBRARY_SUFFIX}")
set(libfake_archive "fake.tar.gz")
add_custom_command(
	OUTPUT ${libfake_archive}
	DEPENDS beholder::fake
	COMMAND
		${CMAKE_COMMAND} -E copy
			${CMAKE_CURRENT_BINARY_DIR}/fake/${libfake}
			${CMAKE_CURRENT_BINARY_DIR}
	COMMAND
		${CMAKE_COMMAND} -E tar cvz ${libfake_archive} ${libfake}
)

add_executable(beholder.test)
target_sources(beholder.test
	PRIVATE
		${libfake_archive}
	PRIVATE
		FILE_SET HEADERS
		BASE_DIRS
			"${CMAKE_CURRENT_SOURCE_DIR}"
		FILES
			Testing.h
			embed.test.h
	PRIVATE
		Testing.cpp
		image.test.cpp
		embed.test.cpp
		neural.test.cpp
)
target_link_libraries(beholder.test
	PRIVATE
		beholder::beholder
		GTest::gtest_main
		beholder::incbin
)
target_compile_definitions(beholder.test
	PRIVATE
		BH_TEST_EMBED_DIR="${CMAKE_CURRENT_BINARY_DIR}"
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

# hacks for improving ASan stack traces and ignoring leaks from pylon libraries
file(WRITE
	"${CMAKE_CURRENT_BINARY_DIR}/bh_cam_tst_lsan.supp"
	"leak:GenApi_3_1_Basler_pylon::CNodeMapFactory"
)

include(GoogleTest)
gtest_discover_tests(beholder.test)
gtest_discover_tests(beholder_camera.test
	PROPERTIES
	ENVIRONMENT "ASAN_OPTIONS=fast_unwind_on_malloc=0"
	ENVIRONMENT "LSAN_OPTIONS=suppressions=${CMAKE_CURRENT_BINARY_DIR}/bh_cam_tst_lsan.supp"
	ENVIRONMENT "PYLON_CAMEMU=3" # make 3 emulated devices available for tests
)
