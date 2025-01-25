cmake_minimum_required(VERSION 3.25.1)
project(beholder-opencv)

FetchContent_Declare(
	bh-opencv-contrib
	GIT_REPOSITORY https://github.com/opencv/opencv_contrib.git
	GIT_TAG 1ed3dd2c53888e3289afdb22ec4e9ebbff3dba87 # 4.10.0
	GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(bh-opencv-contrib)

set(CMAKE_ARGS ${bh_cmake_args})
list(APPEND CMAKE_ARGS
	"-DCMAKE_POLICY_DEFAULT_CMP0069=NEW"
	"-DBUILD_PERF_TESTS=FALSE"
	"-DBUILD_TESTS=FALSE"
	"-DBUILD_opencv_apps=FALSE"
	"-DBUILD_opencv_java=FALSE"
	"-DBUILD_opencv_python3=FALSE"
	"-DENABLE_FLAKE8=FALSE"
	"-DENABLE_PYLINT=FALSE"
	"-DOPENCV_DNN_CUDA=${beholder_USE_CUDA}"
	"-DOPENCV_ENABLE_LTO=${CMAKE_INTERPROCEDURAL_OPTIMIZATION}"
	"-DOPENCV_EXTRA_MODULES_PATH=${bh-opencv-contrib_SOURCE_DIR}/modules"
	"-DOPENCV_FORCE_3RDPARTY_BUILD=TRUE"
	"-DOPENCV_GENERATE_PKGCONFIG=TRUE"
	"-DWITH_CUDA=${beholder_USE_CUDA}"
	"-DOPENCV_3P_LIB_INSTALL_PATH=${CMAKE_INSTALL_LIBDIR}"
)

ExternalProject_Add(
	bh-opencv
	GIT_REPOSITORY https://github.com/opencv/opencv.git
	GIT_TAG 71d3237a093b60a27601c20e9ee6c3e52154e8b1 # 4.10.0
	GIT_SHALLOW TRUE
	UPDATE_DISCONNECTED TRUE
	PATCH_COMMAND git apply "${CMAKE_CURRENT_SOURCE_DIR}/opencv.patch"
	INSTALL_DIR "${bh_staging_prefix}"
	CMAKE_ARGS "${CMAKE_ARGS}"
		-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
		-DCMAKE_PREFIX_PATH=<INSTALL_DIR>
)

ExternalProject_Add_Step(
	bh-opencv update-pkgconfig
	COMMENT "Updating bh-opencv pkg-config file"
	COMMAND
		${CMAKE_COMMAND}
			-Dbh_staging_prefix=${bh_staging_prefix}
			-Dbh_pkgcfg_path=${bh_staging_prefix}/lib/pkgconfig/opencv4.pc
			-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
			-P ${CMAKE_SOURCE_DIR}/cmake/bhUpdatePkgCfg.cmake
	DEPENDEES install
)
