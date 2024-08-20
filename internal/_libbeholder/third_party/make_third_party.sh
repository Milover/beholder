#!/bin/sh

# run from this directory
cd ${0%/*} || exit 1

# exit on error
set -e

#------------------------------------------------------------------------------
# setup

# system check
PLATFORM="$(uname)"

# third party library versions
OPENCV_VERSION="4.10.0"
LEPTONICA_VERSION="1.84.1"
TESSERACT_VERSION="5.4.1"
PYLON_VERSION="7.5.0.15658_linux-x86_64"

# common file paths
THIRD_PARTY_DIR="$PWD"
INSTALL_DIR="/usr/local"
BUILD_SHARED=OFF # TODO: this should be toggleable

OPT_INSTALL_DIR="$INSTALL_DIR"
if [ "$BUILD_SHARED" = "OFF" ]; then
	OPT_INSTALL_DIR="$THIRD_PARTY_DIR/.."
fi

CXX_FLAGS="-march=native -O3"
CC_FLAGS="-march=native -O3"

#------------------------------------------------------------------------------
# clean the install manifest

rm -f install_manifest.txt

#------------------------------------------------------------------------------
# download sources and build archive if necessary

if [ ! -f "archive.tar.gz" ]; then
	mkdir archive
	cd archive

	git clone --depth=1 --branch "$OPENCV_VERSION" https://github.com/opencv/opencv.git
	git clone --depth=1 --branch "$LEPTONICA_VERSION" https://github.com/DanBloomberg/leptonica.git
	git clone --depth=1 --branch "$TESSERACT_VERSION" https://github.com/tesseract-ocr/tesseract.git

	mkdir -v pylon
	curl -O "https://www2.baslerweb.com/media/downloads/software/pylon_software/pylon-${PYLON_VERSION}_setup.tar.gz"
	tar -xvf "pylon-${PYLON_VERSION}_setup.tar.gz"
	tar -C pylon -xvf "pylon-${PYLON_VERSION}.tar.gz"
	rm -vf "pylon-${PYLON_VERSION}.tar.gz" \
		   "pylon-${PYLON_VERSION}_setup.tar.gz" \
		   INSTALL

	cd "$THIRD_PARTY_DIR"

	tar -cvzf archive.tar.gz archive
	#rm -r archive
fi

if [ ! -d "archive" ]; then
	tar -xvzf archive.tar.gz
fi

#------------------------------------------------------------------------------
# Build OpenCV

if [ ! -d opencv ]; then
	echo "setting up OpenCV"

	mv archive/opencv ./
	cd opencv
	git apply "$THIRD_PARTY_DIR/patches/opencv.patch"
	mkdir build
	cd build
	cmake -DCMAKE_BUILD_TYPE=Release \
		  -DCMAKE_CXX_FLAGS="$CXX_FLAGS" \
		  -DCMAKE_C_FLAGS="$CC_FLAGS" \
		  -DCMAKE_INSTALL_PREFIX="$OPT_INSTALL_DIR" \
		  -DBUILD_SHARED_LIBS=$BUILD_SHARED \
		  -DOPENCV_GENERATE_PKGCONFIG=ON \
		  -DOPENCV_FORCE_3RDPARTY_BUILD=ON \
		  ../
	make -j4
	make install

	cat install_manifest.txt >> "$THIRD_PARTY_DIR/install_manifest.txt"
	echo "" >> "$THIRD_PARTY_DIR/install_manifest.txt"

	cd "$THIRD_PARTY_DIR"
else
	echo "OpenCV already set up --- skipping"
fi

#------------------------------------------------------------------------------
# Build Leptonica

if [ ! -d leptonica ]; then
	echo "setting up leptonica"

	mv archive/leptonica ./
	cd leptonica
	git apply "$THIRD_PARTY_DIR/patches/leptonica.patch"
	mkdir build
	cd build
	cmake -DCMAKE_BUILD_TYPE=Release \
		  -DCMAKE_C_FLAGS="$CC_FLAGS" \
		  -DCMAKE_INSTALL_PREFIX="$OPT_INSTALL_DIR" \
		  -DBUILD_SHARED_LIBS=$BUILD_SHARED \
		  -DPNG_LIBRARY="$THIRD_PARTY_DIR/opencv/build/3rdparty/lib/liblibpng.a" \
		  -DPNG_PNG_INCLUDE_DIR="$THIRD_PARTY_DIR/opencv/3rdparty/libpng" \
		  -DZLIB_LIBRARY="$THIRD_PARTY_DIR/opencv/build/3rdparty/lib/libzlib.a" \
		  -DZLIB_INCLUDE_DIR="$THIRD_PARTY_DIR/opencv/3rdparty/zlib" \
		  -DTIFF_LIBRARY="$THIRD_PARTY_DIR/opencv/build/3rdparty/lib/liblibtiff.a" \
		  -DTIFF_INCLUDE_DIR:PATH="$THIRD_PARTY_DIR/opencv/3rdparty/libtiff;$THIRD_PARTY_DIR/opencv/build/3rdparty/libtiff" \
		  -DJPEG_LIBRARY:FILEPATH="$THIRD_PARTY_DIR/opencv/build/3rdparty/lib/liblibjpeg-turbo.a" \
		  -DJPEG_INCLUDE_DIR:PATH="$THIRD_PARTY_DIR/opencv/3rdparty/libjpeg-turbo/src;$THIRD_PARTY_DIR/opencv/build/3rdparty/libjpeg-turbo" \
		  -DWebP_LIBRARY="$THIRD_PARTY_DIR/opencv/build/3rdparty/lib/liblibwebp.a" \
		  -DWebP_INCLUDE_DIR="$THIRD_PARTY_DIR/opencv/3rdparty/libwebp/src" \
		  -DWebP_MUX_LIBRARY="$THIRD_PARTY_DIR/opencv/build/3rdparty/lib/liblibwebp.a" \
		  -DWebP_MUX_INCLUDE_DIR="$THIRD_PARTY_DIR/opencv/3rdparty/libwebp/src" \
		  -DOPENJPEG_LIBRARIES:FILEPATH="$THIRD_PARTY_DIR/opencv/build/3rdparty/lib/liblibopenjp2.a" \
		  -DOPENJPEG_INCLUDE_DIRS:PATH="$THIRD_PARTY_DIR/opencv/3rdparty/openjpeg/openjp2;$THIRD_PARTY_DIR/opencv/build/3rdparty/openjpeg/openjp2" \
		  -DENABLE_GIF=OFF \
		  ../
	make -j4
	make install

	cat install_manifest.txt >> "$THIRD_PARTY_DIR/install_manifest.txt"
	echo "" >> "$THIRD_PARTY_DIR/install_manifest.txt"

	cd "$THIRD_PARTY_DIR"
else
	echo "leptonica already set up --- skipping"
fi

#------------------------------------------------------------------------------
# Build tesseract

if [ ! -d tesseract ]; then
	echo "setting up tesseract"

	mv archive/tesseract ./
	mkdir tesseract/build
	cd tesseract/build
	cmake -DCMAKE_BUILD_TYPE=Release \
		  -DCMAKE_INSTALL_PREFIX="$OPT_INSTALL_DIR" \
		  -DBUILD_SHARED_LIBS=$BUILD_SHARED \
		  -DENABLE_NATIVE=ON \
		  -DBUILD_TRAINING_TOOLS=OFF \
		  -DDISABLE_ARCHIVE=ON \
		  -DINSTALL_CONFIGS=OFF \
		  -DDISABLE_CURL=ON \
		  -DGRAPHICS_DISABLED=ON \
		  -DOPENMP_BUILD=OFF \
		  -DTIFF_INCLUDE_DIR="$THIRD_PARTY_DIR/opencv/3rdparty/libtiff;$THIRD_PARTY_DIR/opencv/build/3rdparty/libtiff" \
		  -DTIFF_LIBRARY="$THIRD_PARTY_DIR/opencv/build/3rdparty/lib/liblibtiff.a" \
		  -DLeptonica_DIR="$OPT_INSTALL_DIR/lib/cmake/leptonica" \
		  ../
	make -j4
	make install

	cat install_manifest.txt >> "$THIRD_PARTY_DIR/install_manifest.txt"
	echo "" >> "$THIRD_PARTY_DIR/install_manifest.txt"

	cd "$THIRD_PARTY_DIR"
else
	echo "tesseract already set up --- skipping"
fi

#------------------------------------------------------------------------------
# Setup pylon

# TODO: we should also check the architecture, but realistically, we'll never
# use an ARM linux, so whatever.
if [ "$PLATFORM" = "Darwin" ]; then
	echo "pylon not available for Apple silicon --- skipping"
elif [ ! -d pylon ]; then
	echo "setting up pylon"

	mv archive/pylon ./
	cd pylon

	# package and 'install' pylon
	# patch cmake files
	sed -i -e 's#/include#/include/pylon#g' \
		   share/pylon/cmake/genicam-targets.cmake \
		   share/pylon/cmake/pylon-targets.cmake \
		   share/pylon/cmake/pylon-targets-release.cmake

	# move cmake files
	mv -v share/pylon/cmake "$INSTALL_DIR/lib/cmake/pylon"
	tree --prune --noreport -f -i "$INSTALL_DIR/lib/cmake/pylon" | sed -e 's/ -> .*//g' | tee -a install_manifest.txt

	# move headers
	mv -v include "$INSTALL_DIR/include/pylon"
	tree --prune --noreport -f -i "$INSTALL_DIR/include/pylon" | sed -e 's/ -> .*//g' | tee -a install_manifest.txt

	# move libraries
	mv -v lib/libpylonbase.so* \
		  lib/libpylonutility.so* \
		  lib/libpylonc.so* \
		  lib/libGCBase_gcc_v3_1_Basler_pylon.so* \
		  lib/libGenApi_gcc_v3_1_Basler_pylon.so* \
		  lib/libLog_gcc_v3_1_Basler_pylon.so* \
		  lib/liblog4cpp_gcc_v3_1_Basler_pylon.so* \
		  lib/libMathParser_gcc_v3_1_Basler_pylon.so* \
		  lib/libNodeMapData_gcc_v3_1_Basler_pylon.so* \
		  lib/libXmlParser_gcc_v3_1_Basler_pylon.so* \
		  lib/libgxapi.so* \
		  lib/libuxapi.so* \
		  lib/libpylon_TL_gige.so* \
		  lib/libpylon_TL_camemu.so* \
		  lib/libpylon_TL_usb.so* \
		  lib/pylon-libusb-1.0.so* \
		  "$INSTALL_DIR/lib/"
	tree --prune --noreport -f -i "$INSTALL_DIR" | sed -e 's/ -> .*//g' | grep -e 'pylon' -e 'Basler' -e 'libgxapi' -e 'libuxapi' | tee -a install_manifest.txt

	# move licenses
	mv -v share/pylon/licenses "$INSTALL_DIR/share/licenses/pylon"
	tree --prune --noreport -f -i "$INSTALL_DIR/share/licenses/pylon" | sed -e 's/ -> .*//g' | tee -a install_manifest.txt

	# make pkg-config file
	tee pylon.pc << EOF
prefix=/usr/local
exec_prefix=\${prefix}
libdir=\${exec_prefix}/lib
includedir=\${prefix}/include/pylon

Name: pylon
Description: Fast and easy image acquisition
Version: ${PYLON_VERSION%.*}
Libs: -L\${exec_prefix}/lib -lpylonc -lpylonutility -lpylonbase -lGenApi_gcc_v3_1_Basler_pylon -lGCBase_gcc_v3_1_Basler_pylon
Libs.private: -lLog_gcc_v3_1_Basler_pylon -lMathParser_gcc_v3_1_Basler_pylon -lXmlParser_gcc_v3_1_Basler_pylon -lNodeMapData_gcc_v3_1_Basler_pylon -ldl -lm -lpthread -lrt
Cflags: -I\${includedir}
EOF
	mv -v pylon.pc "$INSTALL_DIR/lib/pkgconfig/"
	echo "$INSTALL_DIR/lib/pkgconfig/pylon.pc" | tee -a install_manifest.txt

	# append install manifest
	cat install_manifest.txt >> "$THIRD_PARTY_DIR/install_manifest.txt"
	echo "" >> "$THIRD_PARTY_DIR/install_manifest.txt"

	cd "$THIRD_PARTY_DIR"
else
	echo "pylon already set up --- skipping"
fi

#------------------------------------------------------------------------------
# cleanup

if [ "$PLATFORM" = "Linux" ]; then
	ldconfig
fi

rm -rf archive

#------------------------------------------------------------------------------
