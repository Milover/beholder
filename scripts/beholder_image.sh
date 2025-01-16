#!/bin/bash

# beholder - Copyright © 2024 Philipp Milovic
#
# SPDX-License-Identifier: Apache-2.0

set -euo pipefail

function usage() {
	echo "This script builds a Docker image containing the beholder binary and runtime libraries."
	echo ""
	echo "Usage: $0 <CMake preset> [CMake args]"
	exit 1
}

BH_ROOT="$(dirname $(dirname $(realpath $0)))"
CMAKE_PRESET=
CMAKE_ARGS=

if [ "$#" -eq 1 ]; then
	CMAKE_PRESET="$1"
elif ["$#" -eq 2 ]; then
	CMAKE_PRESET="$1"
	CMAKE_ARGS="$2"
else
	usage
fi

docker build \
	--file="$BH_ROOT/build/ci/beholder.Dockerfile" \
	--build-arg bh_cmake_preset="$CMAKE_PRESET" \
	--build-arg bh_cmake_args="$CMAKE_ARGS" \
	--tag="beholder:latest" \
	--target=runtime \
	--network=host \
	"$BH_ROOT"
