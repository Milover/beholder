#!/bin/bash

# beholder - Copyright © 2024 Philipp Milovic
#
# SPDX-License-Identifier: Apache-2.0

set -euo pipefail

# Constants
BH_ROOT="$(dirname $(dirname $(realpath $0)))"

function usage() {
	echo "This script cleans the beholder build (Go code only)."
	echo ""
	echo "Go build tags can optionally be provided as a single comma-delimited"
	echo "string."
	echo ""
	echo "Usage: $0 [go-build-tags]"
	echo ""
	echo "Example: $0 \"tag1,tag2\""
	exit 1
}

build_tags="${1:-""}"

cd "$BH_ROOT"

# TODO: clean generated files
go clean -tags="$build_tags" ./...
rm -rf bin
