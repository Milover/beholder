// Copyright © 2024 P. Milovic
// SPDX-License-Identifier: Apache-2.0

#ifndef SRC_DIR
#error "SRC_DIR not defined"
#endif

#include <incbin/incbin.h>

// NOTE: std::filesystem::path is not constexpr, so we can't use it here.
INCBIN(Lorem, SRC_DIR "/testdata/loremipsum.txt");
INCBIN(Onebyte, SRC_DIR "/testdata/onebyte.txt");
INCBIN(Sevenbytes, SRC_DIR "/testdata/sevenbytes.txt");

INCBIN(char, CharLorem, SRC_DIR "/testdata/loremipsum.txt");
INCBIN(char, CharOnebyte, SRC_DIR "/testdata/onebyte.txt");
INCBIN(char, CharSevenbytes, SRC_DIR "/testdata/sevenbytes.txt");

INCTXT(TLorem, SRC_DIR "/testdata/loremipsum.txt");
