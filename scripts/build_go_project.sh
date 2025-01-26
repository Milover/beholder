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
	echo "Usage: $0 [go-flags]"
	echo ""
	echo "Example: $0 \"-asan -tags=tag1,tag2\""
	exit 1
}

go_flags="${1:-""}"
target=$(basename $(go list -m))

cd "$BH_ROOT"

go generate -x "$go_flags" ./...
go mod tidy
go build "$go_flags" -o bin/$target main.go
