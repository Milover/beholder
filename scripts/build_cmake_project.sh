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
	cat<<EOF
Build and install a CMake project.
Used to build and install the C-API or it's third-party dependencies.

The project is installed into 'CMAKE_PREFIX' (environment variable), which
defaults to '/usr/local' if not set.
An override can be supplied as a command line argument through [cmake-prefix].

Usage:
    scrips/${0##*/} <cmake-project-dir> <cmake-preset> [cmake-prefix]

Example:
    scripts/${0##*/} _c-api debug _c-api/build/staging

EOF
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
