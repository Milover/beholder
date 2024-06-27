#!/bin/sh

# run from this directory
cd ${0%/*} || exit 1

# exit on error
set -e

#------------------------------------------------------------------------------

OPENCV_VERSION="4.9.0"
LEPTONICA_VERSION="1.84.1"
TESSERACT_VERSION="5.3.4"

THIRD_PARTY_DIR="$PWD"
DESTINATION_DIR="$PWD/../share"

#------------------------------------------------------------------------------
# Build OpenCV

if [ ! -d opencv ]; then
	echo "setting up OpenCV"

	git clone --depth=1 --branch "$OPENCV_VERSION" https://github.com/opencv/opencv.git
	cd opencv
	mkdir build
	cd build
	cmake -DCMAKE_BUILD_TYPE=Release \
		  -DBUILD_SHARED_LIBS=OFF \
		  -DCMAKE_INSTALL_PREFIX="$DESTINATION_DIR" \
		  -DOPENCV_FORCE_3RDPARTY_BUILD=ON \
		  ../
	make -j4
	make install

	cd "$THIRD_PARTY_DIR"
else
	echo "OpenCV already set up --- skipping"
fi

#------------------------------------------------------------------------------
# Build Leptonica

if [ ! -d leptonica ]; then
	echo "setting up leptonica"

	git clone --depth=1 --branch "$LEPTONICA_VERSION" https://github.com/DanBloomberg/leptonica.git
	cd leptonica
	git apply "../patches/leptonica.patch"
	mkdir build
	cd build
	cmake -DCMAKE_BUILD_TYPE=Release \
		  -DCMAKE_INSTALL_PREFIX="$DESTINATION_DIR" \
		  -DBUILD_SHARED_LIBS=OFF \
		  -DPNG_LIBRARY="$DESTINATION_DIR/lib/opencv4/3rdparty/liblibpng.a" \
		  -DPNG_PNG_INCLUDE_DIR="$THIRD_PARTY_DIR/opencv/3rdparty/libpng" \
		  -DZLIB_LIBRARY="$DESTINATION_DIR/lib/opencv4/3rdparty/libzlib.a" \
		  -DZLIB_INCLUDE_DIR="$THIRD_PARTY_DIR/opencv/3rdparty/zlib" \
		  -DTIFF_LIBRARY="$DESTINATION_DIR/lib/opencv4/3rdparty/liblibtiff.a" \
		  -DTIFF_INCLUDE_DIR:PATH="$THIRD_PARTY_DIR/opencv/3rdparty/libtiff;$THIRD_PARTY_DIR/opencv/build/3rdparty/libtiff" \
		  -DJPEG_LIBRARY:FILEPATH="$DESTINATION_DIR/lib/opencv4/3rdparty/liblibjpeg-turbo.a" \
		  -DJPEG_INCLUDE_DIR:PATH="$THIRD_PARTY_DIR/opencv/3rdparty/libjpeg-turbo/src;$THIRD_PARTY_DIR/opencv/build/3rdparty/libjpeg-turbo" \
		  -DWebP_LIBRARY="$DESTINATION_DIR/lib/opencv4/3rdparty/liblibwebp.a" \
		  -DWebP_INCLUDE_DIR="$THIRD_PARTY_DIR/opencv/3rdparty/libwebp/src" \
		  -DWebP_MUX_LIBRARY="$DESTINATION_DIR/lib/opencv4/3rdparty/liblibwebp.a" \
		  -DWebP_MUX_INCLUDE_DIR="$THIRD_PARTY_DIR/opencv/3rdparty/libwebp/src" \
		  -DOPENJPEG_LIBRARIES:FILEPATH="$DESTINATION_DIR/lib/opencv4/3rdparty/liblibopenjp2.a" \
		  -DOPENJPEG_INCLUDE_DIRS:PATH="$THIRD_PARTY_DIR/opencv/3rdparty/openjpeg/openjp2;$THIRD_PARTY_DIR/opencv/build/3rdparty/openjpeg/openjp2" \
		  -DENABLE_GIF=OFF \
		  ../
	make -j4
	make install

	cd "$THIRD_PARTY_DIR"
else
	echo "leptonica already set up --- skipping"
fi

#------------------------------------------------------------------------------
# Build tesseract

if [ ! -d tesseract ]; then
	echo "setting up tesseract"

	git clone --depth=1 --branch "$TESSERACT_VERSION" https://github.com/tesseract-ocr/tesseract.git
	cd tesseract
	mkdir build
	cd build
	cmake -DCMAKE_BUILD_TYPE=Release \
		  -DCMAKE_INSTALL_PREFIX="$DESTINATION_DIR" \
		  -DBUILD_SHARED_LIBS=OFF \
		  -DINSTALL_CONFIGS=OFF \
		  -DBUILD_TRAINING_TOOLS=OFF \
		  -DDISABLE_CURL=ON \
		  -DDISABLE_ARCHIVE=ON \
		  -DTIFF_INCLUDE_DIR="$THIRD_PARTY_DIR/opencv/3rdparty/libtiff;$THIRD_PARTY_DIR/opencv/build/3rdparty/libtiff;$DESTINATION_DIR/lib/opencv4/3rdparty" \
		  -DTIFF_LIBRARY="$DESTINATION_DIR/lib/opencv4/liblibtiff.a" \
		  -DLeptonica_DIR="$DESTINATION_DIR/lib/cmake/leptonica" \
		  ../
	make -j4
	make install

	cd "$THIRD_PARTY_DIR"
else
	echo "tesseract already set up --- skipping"
fi

#------------------------------------------------------------------------------
