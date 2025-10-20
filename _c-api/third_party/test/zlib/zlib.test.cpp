// Copyright © 2024 P. Milovic
// SPDX-License-Identifier: Apache-2.0

// Compile and basic operation test for the zlib library.

#include <gtest/gtest.h>

namespace zlib {

#include <zconf.h>
#include <zlib.h>

}  // namespace zlib

#include <string>

// NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)

// Compress and uncompress a local file using zlib and check the contents
// before and after.
TEST(zlib, CompressUncompress) {  // NOLINT
	using ByteStr = std::basic_string<zlib::Bytef>;

	const ByteStr expected{reinterpret_cast<const unsigned char*>("TEST")};

	zlib::uLong bound{zlib::compressBound(expected.size())};
	ByteStr compressed(bound, '\0');
	const int compRes{zlib::compress2(compressed.data(), &bound,
									  expected.data(), expected.size(),
									  Z_BEST_COMPRESSION)};
	EXPECT_EQ(compRes, Z_OK);

	zlib::uLong decompSize{expected.size()};
	ByteStr decompressed(expected.size(), '\0');  // NOLINT(*-const-correctness)
	const int decompRes{zlib::uncompress(decompressed.data(), &decompSize,
										 compressed.data(), compressed.size())};
	EXPECT_EQ(decompRes, Z_OK);

	EXPECT_EQ(decompressed, expected)
		<< "got: " << reinterpret_cast<const char*>(decompressed.data());
}

// NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
