#!/bin/sh

OPENCV_VERSION="4.9.0"

# Build OpenCV

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
