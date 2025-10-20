// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BEHOLDER_TEST_EMBED_TEST_H
#define BEHOLDER_TEST_EMBED_TEST_H

#include <beholder/embed/Embed.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <string>
#include <system_error>

namespace beholder {
namespace test {

// errString returns a pretty-printed error category, code and message.
std::string errString(const std::error_code& err);

// readFile reads a file into the provided buffer.
// The buffer is resized to the file size.
//
// If there is an error, the function returns testing::AssertionFailure,
// otherwise testing::AssertionSuccess is returned.
testing::AssertionResult
readFile(const std::filesystem::path& file, embed::ByteVector& buf);

}  // namespace test
}  // namespace beholder

#endif	// BEHOLDER_TEST_EMBED_TEST_H
