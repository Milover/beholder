// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BEHOLDER_EMBED_MANAGER_H
#define BEHOLDER_EMBED_MANAGER_H

#include <cstddef>
#include <filesystem>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

namespace beholder {
namespace embed {

using CByteSpan = std::span<const unsigned char>;
using PathVector = std::vector<std::filesystem::path>;

class Manager {
private:
	std::filesystem::path outdir_;	// binary resource output directory
	PathVector files_;				// binary resources under management

	// unpack is a helper which handles zlib/gzip decompression and tar
	// unarchiving.
	void unpack(CByteSpan bin) noexcept;

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
	explicit Manager(CByteSpan bin, std::filesystem::path outdir = {}) noexcept;

	// Overload for easier use with symbols provided by INCBIN.
	//
	// NOLINTNEXTLINE(*-c-arrays)
	Manager(const unsigned char (&bin)[], size_t binSize,
			std::filesystem::path outdir = {}) noexcept
		: Manager({static_cast<std::decay_t<decltype(bin)>>(bin), binSize},
				  std::move(outdir)) {}

	Manager(const Manager&) = delete;
	Manager(Manager&&) = delete;

	// Recursively removes the output directory.
	~Manager() noexcept;

	Manager& operator=(const Manager&) = delete;
	Manager& operator=(Manager&&) = delete;

	[[nodiscard]] std::filesystem::path getOutDir() const { return outdir_; }
	[[nodiscard]] const PathVector& getFiles() const { return files_; }
};

}  // namespace embed
}  // namespace beholder

#endif	// BEHOLDER_EMBED_MANAGER_H
