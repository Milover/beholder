#!/bin/bash

# beholder - Copyright © 2024 Philipp Milovic
#
# SPDX-License-Identifier: Apache-2.0

set -euo pipefail

# Constants
BH_ROOT="$(dirname $(dirname $(realpath $0)))"

function usage() {
	echo "This script builds a Docker image containing the beholder binary and"
	echo "runtime libraries."
	echo ""
	echo "Usage: $0 <cmake-preset>"
	exit 1
}

if [ "$#" -ne 1 ]; then
	usage
fi

cmake_preset="$1"

docker build \
	--file="$BH_ROOT/build/ci/beholder.Dockerfile" \
	--build-arg cmake_preset="$CMAKE_PRESET" \
	--tag="beholder:latest" \
	--target=runtime \
	--network=host \
	"$BH_ROOT"
