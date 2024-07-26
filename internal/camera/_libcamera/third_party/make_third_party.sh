#!/bin/sh

# run from this directory
cd ${0%/*} || exit 1

# exit on error
set -e

#------------------------------------------------------------------------------

PYLON_VERSION="7.5.0.15658_linux-x86_64"

THIRD_PARTY_DIR="$PWD"
INSTALL_DIR="/usr/local"

#------------------------------------------------------------------------------
# Setup pylon

# TODO: we should also check the architecture, but realistically, we'll never
# use an ARM linux, so whatever.
if [ "$OSTYPE" = "darwin" ]; then
	echo "pylon not available for Apple silicon --- skipping"
elif [ ! -d pylon ]; then
	echo "setting up pylon"

	mkdir -v pylon
	cd pylon
	curl -O "https://www2.baslerweb.com/media/downloads/software/pylon_software/pylon-${PYLON_VERSION}_setup.tar.gz"
	tar -xvf "pylon-${PYLON_VERSION}_setup.tar.gz"
	tar -xvf "pylon-${PYLON_VERSION}.tar.gz"
	rm -vf "pylon-${PYLON_VERSION}_setup.tar.gz"

	# package and 'install' pylon
	# patch cmake files
	sed -i -e 's#/include#/include/pylon#g' \
		   share/pylon/cmake/genicam-targets.cmake \
		   share/pylon/cmake/pylon-targets.cmake \
		   share/pylon/cmake/pylon-targets-release.cmake

	# move cmake files
	mv -v share/pylon/cmake "$INSTALL_DIR/lib/cmake/pylon"

	# move headers
	mv -v include "$INSTALL_DIR/include/pylon"

	# move libraries
	mv -v lib/libpylonbase.so* \
		  lib/libpylonutility.so* \
		  lib/libpylonc.so* \
		  lib/libGCBase_gcc_v3_1_Basler_pylon.so* \
		  lib/libGenApi_gcc_v3_1_Basler_pylon.so* \
		  lib/libLog_gcc_v3_1_Basler_pylon.so* \
		  lib/liblog4cpp_gcc_v3_1_Basler_pylon.so* \
		  lib/libMathParser_gcc_v3_1_Basler_pylon.so* \
		  lib/libXmlParser_gcc_v3_1_Basler_pylon.so* \
		  lib/libNodeMapData_gcc_v3_1_Basler_pylon.so* \
		  lib/libgxapi.so* \
		  lib/libpylon_TL_gige.so* \
		  lib/libuxapi.so* \
		  lib/pylon-libusb-1.0.so* \
		  lib/libpylon_TL_usb.so* \
		  lib/gentlproducer \
		  "$INSTALL_DIR/lib/"

	# move licenses
	mv -v share/pylon/licenses "$INSTALL_DIR/share/licenses/pylon"

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

	# make install manifest
	tree --prune --noreport -f -i "$INSTALL_DIR" | sed -e 's/ -> .*//g' | grep -e 'pylon' -e 'gentlproducer' -e 'Basler' -e 'libgxapi' -e 'libuxapi' | tee -a install_manifest.txt
	cat install_manifest.txt >> "$THIRD_PARTY_DIR/install_manifest.txt"

	cd "$THIRD_PARTY_DIR"
else
	echo "pylon already set up --- skipping"
fi

#------------------------------------------------------------------------------
