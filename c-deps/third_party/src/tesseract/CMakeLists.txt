cmake_minimum_required(VERSION 3.25.1)
project(beholder-tesseract)

ExternalProject_Get_Property(bh-opencv SOURCE_DIR)
set(ocv_SOURCE_DIR "${SOURCE_DIR}")
ExternalProject_Get_Property(bh-opencv BINARY_DIR)
set(ocv_BINARY_DIR "${BINARY_DIR}")

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
	"-DTIFF_LIBRARY=${ocv_BINARY_DIR}/3rdparty/lib/liblibtiff.a"
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
