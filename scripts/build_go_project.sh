#!/bin/bash

# beholder - Copyright © 2024 Philipp Milovic
#
# SPDX-License-Identifier: Apache-2.0

set -euo pipefail

# Constants
BH_ROOT="$(dirname $(dirname $(realpath $0)))"

function usage() {
	echo "This script builds the beholder binary."
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
target=$(basename $(go list -m))

cd "$BH_ROOT"

go mod tidy
go generate -tags="$build_tags" ./...
go build -tags="$build_tags" -o bin/$bh_target main.go
