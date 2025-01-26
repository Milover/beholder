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
	echo "Usage: $0 [go-flags]"
	echo ""
	echo "Example: $0 \"-x -cache\""
	exit 1
}

go_flags="${1:-}"

cd "$BH_ROOT"

# TODO: clean generated files
go clean $go_flags ./...
rm -rf bin
