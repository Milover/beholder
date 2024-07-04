#!/bin/sh

# run from this directory
cd ${0%/*} || exit 1

# exit on error
set -e

#------------------------------------------------------------------------------

PYLON_VERSION="7.5.0.15658_linux-x86_64"

THIRD_PARTY_DIR="$PWD"
DESTINATION_DIR="$PWD/../share"

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
	rm -vf pylon-*.tar.gz INSTALL

	# package and 'install' pylon
	# patch cmake files
	sed -i -e 's#/lib/#/lib/pylon/#g' -e 's#/include#/include/pylon#g' \
		   share/pylon/cmake/genicam-targets.cmake \
		   share/pylon/cmake/pylon-targets.cmake \
		   share/pylon/cmake/pylon-targets-release.cmake

	# move cmake files
	mv -v share/pylon/cmake "$DESTINATION_DIR/lib/cmake/pylon"

	# move headers
	mv -v include "$DESTINATION_DIR/include/pylon"

	# move libraries
	mkdir -v "$DESTINATION_DIR/lib/pylon"
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
		  "$DESTINATION_DIR/lib/pylon/"

	# move licenses
	mv -v share/pylon/licenses "$DESTINATION_DIR/share/licenses/pylon"

	cd "$THIRD_PARTY_DIR"
else
	echo "pylon already set up --- skipping"
fi

#------------------------------------------------------------------------------
