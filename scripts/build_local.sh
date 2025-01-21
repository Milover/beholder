#!/bin/bash

# beholder - Copyright © 2024 Philipp Milovic
#
# SPDX-License-Identifier: Apache-2.0

# Build beholder third-party dependencies, the C-library and the final binary,
# using a local staging directory.

set -euo pipefail

function usage() {
	echo "This script builds and installs beholder's third-party dependencies,"
	echo "C-libraries and the final binary using a local staging directory."
	echo ""
	echo "Usage: $0 <CMake preset>"
	exit 1
}

function notice() {
	echo "NOTE:"
	echo "    Building beholder third-party dependencies and C-libraries into"
	echo "    staging directory:"
	echo ""
	echo "        $1"
	echo ""
	echo "    To run the final binary, set or export 'LD_LIBRARY_PATH' to:"
	echo ""
	echo "        $1/lib"
	echo ""
	echo "    or preload the shared libraries with 'LD_PRELOAD'."
	echo ""
}

CMAKE_PRESET="release"

if [ "$#" -eq 1 ]; then
	CMAKE_PRESET="$1"
elif [ "$#" -ge 2 ]; then
	usage
fi

BH_ROOT="$(dirname $(dirname $(realpath $0)))"
BH_BUILD="$BH_ROOT/_c-api/build/$CMAKE_PRESET"
BH_3P_BUILD="$BH_ROOT/_c-api/third_party/build/$CMAKE_PRESET"
BH_STAGING="$BH_ROOT/_c-api/build/staging"


# build beholder C-library third-party dependencies
function build_third_party() {
	local cwd="$(pwd)"

	cd "$BH_ROOT/_c-api/third_party"
	rm -rf "$BH_3P_BUILD"

	cmake --preset="$CMAKE_PRESET" \
		-DCMAKE_INSTALL_PREFIX="$BH_STAGING"
	cmake --workflow --preset="$CMAKE_PRESET"

	cd "$cwd"
}

# build beholder C-libraries
function build_libs() {
	local cwd="$(pwd)"

	cd "$BH_ROOT/_c-api"
	rm -rf "$BH_BUILD" "$BH_STAGING"
	mkdir -p "$BH_STAGING"

	# unpack third-third party stuff and grab MD5
	 local bh_tp_pkg="$( \
	 	ls -1 "$BH_3P_BUILD" | \
		grep -e 'beholder-third-party-.*\.tar\.gz$' | \
		sed -e 's/\(.*\)\.tar\.gz/\1/g' \
	)"
	local bh_tp_archive="$bh_tp_pkg.tar.gz"
	local bh_tp_md5="$( \
		cat "$BH_3P_BUILD/$bh_tp_archive.md5" | \
		awk '{$print $1}' \
	)"
	tar -cvzf "$BH_3P_BUILD/$bh_tp_archive"
	mv $bh_tp_archive/* "$BH_STAGING/"
	rmdir "$bh_tp_archive"

	# build the library
	cmake --preset="$CMAKE_PRESET" \
		-DCMAKE_INSTALL_PREFIX="$BH_STAGING" \
		-Dbh_third_party_pkg="$BH_3P_BUILD/$bh_tp_archive" \
		-Dbh_third_party_md5="$bh_tp_md5"
	cmake --build --preset="$CMAKE_PRESET"
	cmake --install "$BH_BUILD"

	cd "$cwd"
}

# build beholder binary
function build_beholder() {
	local cwd="$(pwd)"

	make clean
	PKG_CONFIG_PATH="$BH_STAGING/lib/pkgconfig" \
		LD_LIBRARY_PATH="$BH_STAGING/lib" \
		LIBRARY_PATH="$BH_STAGING/lib" \
		make build

	cd "$cwd"
}

function main() {
	notice "$BH_STAGING"

	echo "building third-party dependencies..."
	build_third_party

	echo "building beholder C-libraries dependencies..."
	build_libs

	echo "building beholder binary..."
	build_beholder
}

main
