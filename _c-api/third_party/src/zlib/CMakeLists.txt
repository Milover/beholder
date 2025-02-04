cmake_minimum_required(VERSION 3.25.1)
project(beholder-zlib)

set(CMAKE_ARGS ${bh_cmake_args})
list(APPEND CMAKE_ARGS
	"-DZLIB_BUILD_TESTING=FALSE"
	"-DZLIB_BUILD_SHARED=FALSE"
	"-DZLIB_BUILD_STATIC=TRUE"
	"-DZLIB_BUILD_MINIZIP=FALSE"
	"-DZLIB_INSTALL=TRUE"
	#"-DENABLE_NATIVE=${CMAKE_INTERPROCEDURAL_OPTIMIZATION}"	# something more appropriate would be nicer
	#"-DENABLE_LTO=${CMAKE_INTERPROCEDURAL_OPTIMIZATION}"
)

ExternalProject_Add(
	bh-zlib
	GIT_REPOSITORY https://github.com/madler/zlib.git
	GIT_TAG ed1610791c0c681ee562bb3997e0f29bfae11b2d # 1.4.1.1-dev @ 2025-02-04
	GIT_SHALLOW TRUE
	UPDATE_DISCONNECTED TRUE
	PATCH_COMMAND git apply ${CMAKE_CURRENT_SOURCE_DIR}/zlib.patch
	INSTALL_DIR "${bh_staging_prefix}"
	LIST_SEPARATOR ","
	CMAKE_ARGS "${CMAKE_ARGS}"
		-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
		-DCMAKE_PREFIX_PATH=<INSTALL_DIR>
)

ExternalProject_Add_Step(
	bh-zlib update-pkgconfig
	COMMENT "Updating bh-zlib pkg-config file"
	COMMAND
		${CMAKE_COMMAND}
			-Dbh_staging_prefix=${bh_staging_prefix}
			-Dbh_pkgcfg_path=${bh_staging_prefix}/lib/pkgconfig/zlib.pc
			-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
			-P ${CMAKE_SOURCE_DIR}/cmake/bhUpdatePkgCfg.cmake
	DEPENDEES install
)
