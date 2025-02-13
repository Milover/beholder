// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BEHOLDER_EMBED_MANAGER_H
#define BEHOLDER_EMBED_MANAGER_H

#include <filesystem>
#include <span>

namespace beholder {
namespace embed {

using ConstByteSpan = std::span<const unsigned char>;

class Manager {
private:
	std::filesystem::path outdir_;

public:
	// Construct from a path to a directory.
	//
	// Binary resources are unarchives and output into the supplied directory,
	// or a freshly created temporary directory if no directory is supplied.
	// The directory (either the supplied one or the temporary one) is
	// recursively removed when the destructor is called.
	// Setting addLdPath to true, prepends the output directory to the dynamic
	// loader search path. See embed::prependToLdPath for more info.
	//
	// If there is an error, std::exit is called with EXIT_FAILURE.
	explicit Manager(ConstByteSpan bin,
					 std::filesystem::path outdir = {}) noexcept;

	Manager(const Manager&) = delete;
	Manager(Manager&&) = delete;

	// Recursively removes the output directory.
	~Manager() noexcept;

	Manager& operator=(const Manager&) = delete;
	Manager& operator=(Manager&&) = delete;

	[[nodiscard]] std::filesystem::path getOutDir() const { return outdir_; }
};

}  // namespace embed
}  // namespace beholder

#endif	// BEHOLDER_EMBED_MANAGER_H
