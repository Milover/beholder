#!/bin/bash

install_name_tool -add_rpath /usr/local/lib "$1"
install_name_tool -add_rpath internal/ocr/libocr/lib "$1"

"$1" "$@"
rm "$1"
