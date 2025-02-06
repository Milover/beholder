// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

// Binary resource embedding tests.

#include "embed.test.h"

#include <beholder/embed/Embed.h>
#include <gtest/gtest.h>

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <system_error>

#include "Testing.h"  // NOLINT

namespace fs = std::filesystem;

namespace beholder {
namespace test {

// Test fixtures and helpers
// -------------------------

testing::AssertionResult
readFile(const fs::path& file, embed::ByteVector& buf) {
	std::error_code err{};
	const size_t fsize{fs::file_size(file, err)};
	if (err) {
		return testing::AssertionFailure()
			   << err.category().name() << " error (" << err.value()
			   << "): " << err.message();
	}
	buf.resize(fsize);

	std::ifstream ifs{file, std::ifstream::binary};
	if (!ifs) {
		return testing::AssertionFailure() << "error openning file: " << file;
	}

	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
	ifs.read(reinterpret_cast<char*>(buf.data()),
			 static_cast<std::streamsize>(buf.size()));
	if (!ifs) {
		return testing::AssertionFailure()
			   << "error after reading " << ifs.gcount() << "/" << fsize
			   << " bytes: " << file;
	}
	return testing::AssertionSuccess();
}

// Tests
// -----

TEST(Embed, UncompressGzip) {  // NOLINT(*-function-cognitive-complexity)
	const auto archive{assetsDir / "directory.tar.gz"};
	const auto expected{assetsDir / "directory.tar"};

	std::cerr << "reading test files" << '\n';
	embed::ByteVector archiveBuf;
	embed::ByteVector expectedBuf;
	ASSERT_TRUE(readFile(archive, archiveBuf));
	ASSERT_TRUE(readFile(expected, expectedBuf));

	std::cerr << "uncompressing: " << archive << '\n';
	const embed::ByteVector uncompressed{embed::decompressGzip(archiveBuf)};

	EXPECT_EQ(uncompressed, expectedBuf);
}

}  // namespace test
}  // namespace beholder
