#!/bin/bash

# beholder - Copyright © 2024 Philipp Milovic
#
# SPDX-License-Identifier: Apache-2.0

set -euo pipefail

# Constants
BH_ROOT="$(dirname $(dirname $(realpath $0)))"

function usage() {
	cat<<EOF
Clean a Go project.
Used to clean the beholder binary build artifacts.

Usage:
    scripts/${0##*/} [go-flags]

Example:
    scripts/${0##*/} "-x -cache"

EOF
	exit 1
}

go_flags="${1:-}"

cd "$BH_ROOT"

# TODO: clean generated files
go clean $go_flags ./...
rm -rf bin
