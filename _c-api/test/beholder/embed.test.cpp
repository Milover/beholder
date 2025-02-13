// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

// Binary resource embedding tests.

#include "embed.test.h"

#include <beholder/embed/Embed.h>
#include <beholder/embed/Manager.h>
#include <beholder/util/ScopeGuard.h>
#include <dlfcn.h>
#include <gtest/gtest.h>
#include <incbin/incbin.h>

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <system_error>

#include "Testing.h"  // NOLINT
#include "fake/Fake.h"

namespace fs = std::filesystem;

INCBIN(FakeArchive, BH_TEST_EMBED_DIR "/fake.tar.gz");

namespace beholder {
namespace test {

// Test fixtures and helpers
// -------------------------

std::string errString(const std::error_code& err) {
	std::stringstream ss;
	ss << err.category().name() << " error (" << err.value()
	   << "): " << err.message() << '\n';
	return ss.str();
}

testing::AssertionResult
readFile(const fs::path& file, embed::ByteVector& buf) {
	std::error_code err{};
	const size_t fsize{fs::file_size(file, err)};
	if (err) {
		return testing::AssertionFailure() << errString(err);
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

TEST(Embed, UncompressGzip) {
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

TEST(Embed, UnarchiveTar) {	 // NOLINT(*-function-cognitive-complexity)
	const auto archive{assetsDir / "directory.tar"};

	std::cerr << "reading test files" << '\n';
	embed::ByteVector archiveBuf;
	ASSERT_TRUE(readFile(archive, archiveBuf));

	const fs::path outDir{fs::temp_directory_path() / std::tmpnam(nullptr)};

	std::cerr << "creating output directory: " << outDir << '\n';
	std::error_code err{};
	fs::create_directories(outDir, err);
	ASSERT_FALSE(err) << errString(err);
	const ScopeGuard g{[&]() noexcept {
		std::error_code e{};
		if (err) {
			std::cerr << "error during test execution; "
					  << "not cleaning up temporary directory: " << outDir
					  << '\n';
		}
		fs::remove_all(outDir, e);
		if (e) {
			std::cerr << errString(e);
		}
	}};

	std::cerr << "unarchiving: " << archive << '\n';
	embed::unarchiveTar(archiveBuf, outDir);

	// TODO: would be nice to embed the structure into the test fixture
	const fs::path expDir{outDir / "directory"};
	const fs::path expFile{outDir / "directory/file.txt"};
	const fs::path expLink{outDir / "directory/link"};
	const fs::path expExe{outDir / "directory/file_0755"};

	std::cerr << "checking directory: " << expDir << '\n';
	auto dirStatus{fs::status(expDir, err)};
	ASSERT_FALSE(err) << errString(err);
	EXPECT_EQ(dirStatus.type(), fs::file_type::directory);

	std::cerr << "checking file: " << expFile << '\n';
	auto fileStatus{fs::status(expFile, err)};
	ASSERT_FALSE(err) << errString(err);
	EXPECT_EQ(fileStatus.type(), fs::file_type::regular);

	std::cerr << "checking link: " << expLink << '\n';
	auto linkStatus{fs::symlink_status(expLink, err)};
	ASSERT_FALSE(err) << errString(err);
	EXPECT_EQ(linkStatus.type(), fs::file_type::symlink);

	const fs::path linkTarget(fs::read_symlink(expLink, err));
	ASSERT_FALSE(err) << errString(err);
	EXPECT_EQ(linkTarget, expFile.filename());

	std::cerr << "checking executable: " << expExe << '\n';
	auto exeStatus{fs::symlink_status(expExe, err)};
	ASSERT_FALSE(err) << errString(err);
	EXPECT_EQ(exeStatus.type(), fs::file_type::regular);

	fs::perms expExePerms = fs::perms::all;
	expExePerms &= ~(fs::perms::group_write | fs::perms::others_write);
	EXPECT_EQ(exeStatus.permissions(), expExePerms);
}

TEST(Embed, Manager) {
	using FakeFn = int (*)();

	// sanity checks
	ASSERT_NE(gFakeArchiveData, nullptr);
	ASSERT_GT(gFakeArchiveSize, 0);

	std::cerr << "unpacking archive" << std::endl;
	const embed::Manager mgr{{&(gFakeArchiveData[0]), gFakeArchiveSize},
							 fs::path{}};

	// TODO: either Manager, or another helper should handle loading/unloading
	// libraries and/or symbols
	std::cerr << "loading library" << std::endl;
	void* libfake{
		dlopen(fs::path{mgr.getOutDir() / "libfake.so"}.c_str(), RTLD_LAZY)};
	ASSERT_NE(libfake, nullptr);

	dlerror();	// clear errors

	std::cerr << "loading symbols" << std::endl;
	FakeFn fCall{reinterpret_cast<FakeFn>(dlsym(libfake, "call"))};	 // NOLINT
	ASSERT_EQ(dlerror(), nullptr);

	std::cerr << "calling" << std::endl;
	EXPECT_EQ(fCall(), fake::Return);

	std::cerr << "unloading library" << std::endl;
	dlclose(libfake);
	ASSERT_EQ(dlerror(), nullptr);
}

}  // namespace test
}  // namespace beholder
