#!/bin/bash

# beholder - Copyright © 2024 Philipp Milovic
#
# SPDX-License-Identifier: Apache-2.0

set -euo pipefail

# Constants
BH_ROOT="$(dirname $(dirname $(realpath $0)))"

function usage() {
	cat<<EOF
Build the beholder project runtime Docker image.

Usage:
    scripts/${0##*/} <cmake-preset> [docker-target]

EOF
	exit 1
}

if [ "$#" -lt 1 ]; then
	usage
fi

cmake_preset="$1"
docker_target="${2:-runtime}"

docker buildx build \
	--file="$BH_ROOT/build/ci/beholder.Dockerfile" \
	--build-arg cmake_preset="$cmake_preset" \
	--tag="beholder:latest" \
	--target="$docker_target" \
	--network=host \
	"$BH_ROOT"
