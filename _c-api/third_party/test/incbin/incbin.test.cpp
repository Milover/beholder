// Copyright © 2024 P. Milovic
// SPDX-License-Identifier: Apache-2.0

// Compile and basic operation test for the incbin library.

#include <gtest/gtest.h>
#include <incbin/incbin.h>

// automagically available from incbin_defs.cpp, generated at configure time
INCBIN_EXTERN(Lorem);
INCBIN_EXTERN(Onebyte);
INCBIN_EXTERN(Sevenbytes);

INCBIN_EXTERN(char, CharLorem);
INCBIN_EXTERN(char, CharOnebyte);
INCBIN_EXTERN(char, CharSevenbytes);

INCTXT_EXTERN(TLorem);

// NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)

// Copy-pasta of the test from the incbin repo.
//
// https://github.com/graphitemaster/incbin/blob/main/test/asserts.c
TEST(incbin, EmbedStuff) {	// NOLINT
	// same translation unit, so we don't have to INCBIN_EXTERN or extern "C"
	EXPECT_EQ(gLoremSize, 962);
	EXPECT_EQ(&gLoremData[gLoremSize],
			  reinterpret_cast<const unsigned char*>(&gLoremEnd));

	EXPECT_EQ(gOnebyteSize, 1);
	EXPECT_EQ(&gOnebyteData[gOnebyteSize],
			  reinterpret_cast<const unsigned char*>(&gOnebyteEnd));

	EXPECT_EQ(gSevenbytesSize, 7);
	EXPECT_EQ(&gSevenbytesData[gSevenbytesSize],
			  reinterpret_cast<const unsigned char*>(&gSevenbytesEnd));

	EXPECT_EQ(gCharLoremSize, 962);
	EXPECT_EQ(&gCharLoremData[gCharLoremSize],
			  reinterpret_cast<const char*>(&gCharLoremEnd));

	EXPECT_EQ(gCharOnebyteSize, 1);
	EXPECT_EQ(&gCharOnebyteData[gCharOnebyteSize],
			  reinterpret_cast<const char*>(&gCharOnebyteEnd));

	EXPECT_EQ(gCharSevenbytesSize, 7);
	EXPECT_EQ(&gCharSevenbytesData[gCharSevenbytesSize],
			  reinterpret_cast<const char*>(&gCharSevenbytesEnd));

	// 1 extra bit for \0, because we INCTEXT-ed
	EXPECT_EQ(gTLoremSize, 963);
	EXPECT_EQ(&gTLoremData[gTLoremSize],
			  reinterpret_cast<const char*>(&gTLoremEnd));
}

// NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
