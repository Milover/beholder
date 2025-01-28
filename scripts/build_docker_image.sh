#!/bin/bash

# beholder - Copyright © 2024 Philipp Milovic
#
# SPDX-License-Identifier: Apache-2.0

set -euo pipefail

# Constants
BH_ROOT="$(dirname $(dirname $(realpath $0)))"

# Defaults
BUILD_MODE="${1:-release}"
DOCKER_TARGET="${2:-runtime}"
IMAGE_NAME="${3:-"beholder:latest"}"

function usage() {
	cat <<EOF
Build the beholder project runtime Docker image.
The default build mode is 'release', the default target is 'runtime' and
the default tag is 'beholder:latest'

Usage:
    scripts/${0##*/} [build-mode] [docker-target] [tag]

Example:
    scripts/${0##*/} debug builder-third-party beholder-third-party:latest

Flags:
    -h    - print usage

EOF
	exit 1
}

while getopts "h" opt; do
	case "$opt" in
		h) usage ;;
		*) usage ;;
	esac
done
shift $((OPTIND-1))

docker buildx build \
	--file="$BH_ROOT/build/ci/beholder.Dockerfile" \
	--build-arg build_mode="$BUILD_MODE" \
	--tag="$IMAGE_NAME" \
	--target="$DOCKER_TARGET" \
	--network=host \
	"$BH_ROOT"
