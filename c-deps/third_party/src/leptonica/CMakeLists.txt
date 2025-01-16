cmake_minimum_required(VERSION 3.25.1)
project(beholder-leptonica)

ExternalProject_Get_Property(bh-opencv SOURCE_DIR)
set(ocv_SOURCE_DIR "${SOURCE_DIR}")
ExternalProject_Get_Property(bh-opencv BINARY_DIR)
set(ocv_BINARY_DIR "${BINARY_DIR}")

set(CMAKE_ARGS ${bh_cmake_args})
list(APPEND CMAKE_ARGS
	"-DCMAKE_POLICY_DEFAULT_CMP0069=NEW"
	"-DCMAKE_POLICY_DEFAULT_CMP0077=NEW"
	"-DENABLE_GIF=FALSE"
	"-DPNG_LIBRARY=liblibpng.a"
	"-DPNG_PNG_INCLUDE_DIR=${ocv_SOURCE_DIR}/3rdparty/libpng"
	"-DZLIB_LIBRARY=libzlib.a"
	"-DZLIB_INCLUDE_DIR=${ocv_SOURCE_DIR}/3rdparty/zlib,${ocv_BINARY_DIR}/3rdparty/zlib"
	"-DTIFF_LIBRARY=liblibtiff.a"
	"-DTIFF_INCLUDE_DIR=${ocv_SOURCE_DIR}/3rdparty/libtiff,${ocv_BINARY_DIR}/3rdparty/libtiff"
	"-DJPEG_LIBRARY=liblibjpeg-turbo.a"
	"-DJPEG_INCLUDE_DIR=${ocv_SOURCE_DIR}/3rdparty/libjpeg-turbo/src,${ocv_BINARY_DIR}/3rdparty/libjpeg-turbo"
	"-DWebP_LIBRARY=liblibwebp.a"
	"-DWebP_INCLUDE_DIR=${ocv_SOURCE_DIR}/3rdparty/libwebp/src"
	"-DWebP_MUX_LIBRARY=liblibwebp.a"
	"-DWebP_MUX_INCLUDE_DIR=${ocv_SOURCE_DIR}/3rdparty/libwebp/src"
	"-DOPENJPEG_LIBRARIES=liblibopenjp2.a"
	"-DOPENJPEG_INCLUDE_DIRS=${ocv_SOURCE_DIR}/3rdparty/openjpeg/openjp2,${ocv_BINARY_DIR}/3rdparty/openjpeg/openjp2"
	#"-DCMAKE_LIBRARY_PATH=${CMAKE_INSTALL_LIBDIR}"
)

ExternalProject_Add(
	bh-leptonica
	GIT_REPOSITORY https://github.com/DanBloomberg/leptonica.git
	GIT_TAG 7e803e73511fbd320f01314c141d35d2b8491dde # 1.84.1
	GIT_SHALLOW TRUE
	UPDATE_DISCONNECTED TRUE
	PATCH_COMMAND git apply ${CMAKE_CURRENT_SOURCE_DIR}/leptonica.patch
	INSTALL_DIR "${bh_staging_prefix}"
	DEPENDS bh-opencv
	LIST_SEPARATOR ","
	CMAKE_ARGS "${CMAKE_ARGS}"
		-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
		-DCMAKE_PREFIX_PATH=<INSTALL_DIR>
)

ExternalProject_Add_Step(
	bh-leptonica update-pkgconfig
	COMMENT "Updating bh-leptonica pkg-config file"
	COMMAND
		${CMAKE_COMMAND}
			-Dbh_staging_prefix=${bh_staging_prefix}
			-Dbh_pkgcfg_path=${bh_staging_prefix}/lib/pkgconfig/lept.pc
			-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
			-P ${CMAKE_SOURCE_DIR}/cmake/bhUpdatePkgCfg.cmake
	DEPENDEES install
)
