#!/bin/bash

# beholder - Copyright © 2024 Philipp Milovic
#
# SPDX-License-Identifier: Apache-2.0

install_name_tool -add_rpath /usr/local/lib "$1"
install_name_tool -add_rpath internal/_libbeholder/lib "$1"

"$1" "$@"
rm "$1"
