#!/bin/bash

# beholder - Copyright © 2024 Philipp Milovic
#
# SPDX-License-Identifier: Apache-2.0

set -euo pipefail

# Constants
BH_ROOT="$(dirname $(dirname $(realpath $0)))"

function usage() {
	cat<<EOF
Clean a CMake project.
Used to clean the C-API, or it's third-party dependencies, build artifacts.

Usage:
    scripts/${0##*/} <cmake-project-dir> <cmake-preset>

Example:
    scripts/${0##*/} _c-api debug

EOF
	exit 1
}

if [ "$#" -ne 2 ]; then
	usage
fi

cmake_dir="$(realpath $1)"
cmake_preset="$2"
build_dir="$cmake_dir/build/$cmake_preset"

if [ -z "$build_dir" ]; then
	exit 1
fi
if [ -f "$build_dir/install_manifest.txt" ]; then
	cat "$build_dir/install_manifest.txt" | xargs rm -vrf
fi
rm -vrf "$build_dir"
