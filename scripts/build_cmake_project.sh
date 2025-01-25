#!/bin/bash

# beholder - Copyright © 2024 Philipp Milovic
#
# SPDX-License-Identifier: Apache-2.0

set -euo pipefail

# Constants
BH_ROOT="$(dirname $(dirname $(realpath $0)))"

# Defaults
CMAKE_PREFIX="$(realpath ${3:-/usr/local})"

function usage() {
	echo "This script builds and installs a CMake project."
	echo "It is used to build/install the C-API or it's third-party dependencies."
	echo ""
	echo "The project is installed into 'CMAKE_PREFIX'."
	echo "'CMAKE_PREFIX' is read from the environment, if it is unset,"
	echo "it is set to '/usr/local'."
	echo "[cmake-prefix] can be supplied as a command-line argument, as an"
	echo "additional override."
	echo ""
	echo "Usage: $0 <cmake-project-dir> <cmake-preset> [cmake-prefix]"
	echo ""
	echo "Example: $0 $BH_ROOT/_c-api debug $BH_ROOT/_c-api/build/staging"
	exit 1
}

if [ "$#" -lt 2 ]; then
	usage
fi

cmake_dir="$(realpath $1)"
cmake_preset="$2"

cd "$cmake_dir"

cmake --preset="$cmake_preset" -DCMAKE_INSTALL_PREFIX="$CMAKE_PREFIX"
cmake --build --preset="$cmake_preset"
cmake --install "$cmake_dir/build/$cmake_preset"
