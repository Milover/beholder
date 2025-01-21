// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

// Stuff shared between tests

#ifndef BEHOLDER_TEST_TESTING_H
#define BEHOLDER_TEST_TESTING_H

#include <filesystem>
#include <source_location>

// Report sanitizer errors.
extern "C" {
void __ubsan_on_report();  // NOLINT
void __asan_on_report();   // NOLINT
void __msan_on_report();   // NOLINT
void __tsan_on_report();   // NOLINT
}  // extern "C"

namespace beholder {
namespace test {

// assetsDir is the absolute path, define w.r.s. to this source file, to the
// directory containing testing assets (images, DNN model files, etc.)
inline static const auto assetsDir{
	std::filesystem::absolute(std::source_location::current().file_name())
		.parent_path()
		.parent_path()
		.parent_path()
		.parent_path() /
	"test/assets"};

}  // namespace test
}  // namespace beholder

#endif	// BEHOLDER_TEST_TESTING_H
