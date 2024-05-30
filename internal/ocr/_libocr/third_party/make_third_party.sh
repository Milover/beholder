#!/bin/sh

OPENCV_VERSION="4.9.0"
LEPTONICA_VERSION="1.84.1"

#
# Build OpenCV
#

git clone --depth=1 --branch "$OPENCV_VERSION" https://github.com/opencv/opencv.git
cd opencv
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release \
	  -DBUILD_SHARED_LIBS=OFF \
	  -DCMAKE_INSTALL_PREFIX=../../../share \
	  ../
make -j4
make install

#
# Build Leptonica
#

git clone --depth=1 --branch "$LEPTONICA_VERSION" https://github.com/DanBloomberg/leptonica.git
cd leptonica
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release \
	  -DCMAKE_INSTALL_PREFIX=../../../share \
	  -DBUILD_SHARED_LIBS=OFF \
	  -DPNG_LIBRARY=../../../share/lib/opencv4/3rdparty/liblibpng.a \
	  -DZLIB_LIBRARY=../../../share/lib/opencv4/3rdparty/libzlib.a \
	  -DTIFF_LIBRARY=../../../share/lib/opencv4/3rdparty/liblibtiff.a \
	  -DJPEG_LIBRARY=../../../share/lib/opencv4/3rdparty/liblibjpeg-turbo.a \
	  -DOPENJPEG_LIBRARIES=../../../share/lib/opencv4/3rdparty/liblibopenjp2.a \
	  -DENABLE_GIF=OFF \
	  -DENABLE_WEBP=ON \
	  -DWebP_LIBRARIES=../../../share/lib/opencv4/3rdparty/liblibwebp.a \
	  ../
make -j4
make install

#
# Build tesseract
#
