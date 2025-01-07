#!/bin/bash

set -euo pipefail

function usage() {
	echo "This script builds a Docker image containing external dependencies of libbeholder."
	echo ""
	echo "Usage: $0 <CMake preset> [CMake args]"
	exit 1
}

if [ "$#" -lt 1 ]; then
	usage
fi

BH_ROOT="$(dirname $(dirname $(dirname $(realpath $0))))"
BH_LIB_ROOT="$(dirname $(dirname $(realpath $0)))"

CMAKE_PRESET="$1"
CMAKE_ARGS="$2"

docker build \
	--file="$BH_ROOT/build/ci/bh_third_party.Dockerfile" \
	--build-arg bh_cmake_preset="$CMAKE_PRESET" bh_cmake_args="$CMAKE_ARGS" \
	--tag="bh-thirdparty-dev" \
	--target=runtime \
	--network=host \
	"$BH_LIB_ROOT"
