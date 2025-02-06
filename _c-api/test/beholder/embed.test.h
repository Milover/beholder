// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include <beholder/embed/Embed.h>
#include <gtest/gtest.h>

#include <filesystem>

namespace beholder {
namespace test {

testing::AssertionResult
readFile(const std::filesystem::path& file, embed::ByteVector& buf);

}  // namespace test
}  // namespace beholder
