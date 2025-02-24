cmake_minimum_required(VERSION 3.25.1)
project(beholder-tesseract)

ExternalProject_Get_Property(bh-opencv SOURCE_DIR)
set(ocv_SOURCE_DIR "${SOURCE_DIR}")
ExternalProject_Get_Property(bh-opencv BINARY_DIR)
set(ocv_BINARY_DIR "${BINARY_DIR}")
ExternalProject_Get_Property(bh-opencv INSTALL_DIR)
set(ocv_INSTALL_DIR "${INSTALL_DIR}")

set(CMAKE_ARGS ${bh_cmake_args})
list(APPEND CMAKE_ARGS
	"-DBUILD_TESTS=FALSE"
	"-DENABLE_NATIVE=${CMAKE_INTERPROCEDURAL_OPTIMIZATION}"	# something more appropriate would be nicer
	"-DENABLE_LTO=${CMAKE_INTERPROCEDURAL_OPTIMIZATION}"
	"-DBUILD_TRAINING_TOOLS=FALSE"
	"-DDISABLE_ARCHIVE=TRUE"
	"-DINSTALL_CONFIGS=FALSE"
	"-DDISABLE_CURL=TRUE"
	"-DGRAPHICS_DISABLED=TRUE"
	"-DOPENMP_BUILD=FALSE"
	"-DTIFF_INCLUDE_DIR=${ocv_SOURCE_DIR}/3rdparty/libtiff,${ocv_BINARY_DIR}/3rdparty/libtiff"
	"-DTIFF_LIBRARY=liblibtiff.a"
	"-DCMAKE_CXX_FLAGS=-L${ocv_INSTALL_DIR}/${CMAKE_INSTALL_LIBDIR}"
	#"-DLeptonica_DIR=${lpt_INSTALL_DIR}"
)

ExternalProject_Add(
	bh-tesseract
	GIT_REPOSITORY https://github.com/tesseract-ocr/tesseract.git
	GIT_TAG 64eab6c457b2337dd690746a5fde5c222b40d5f8 # 5.5.0
	GIT_SHALLOW TRUE
	UPDATE_DISCONNECTED TRUE
	INSTALL_DIR "${bh_staging_prefix}"
	DEPENDS bh-opencv bh-leptonica
	LIST_SEPARATOR ","
	CMAKE_ARGS "${CMAKE_ARGS}"
		-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
		-DCMAKE_PREFIX_PATH=<INSTALL_DIR>
)

ExternalProject_Add_Step(
	bh-tesseract update-pkgconfig
	COMMENT "Updating bh-tesseract pkg-config file"
	COMMAND
		${CMAKE_COMMAND}
			-Dbh_staging_prefix=${bh_staging_prefix}
			-Dbh_pkgcfg_path=${bh_staging_prefix}/lib/pkgconfig/tesseract.pc
			-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
			-P ${CMAKE_SOURCE_DIR}/cmake/bhUpdatePkgCfg.cmake
	DEPENDEES install
)
