// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BEHOLDER_EMBED_UNPACKER_H
#define BEHOLDER_EMBED_UNPACKER_H

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

class Unpacker {
private:
	std::filesystem::path outdir_;	// binary resource output directory
	PathVector files_;				// binary resources under management
	bool cleanup_{true};			// remove output directory on destructor

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
	//
	// If there is an error, std::exit is called with EXIT_FAILURE.
	explicit Unpacker(CByteSpan bin,
					  std::filesystem::path outdir = {}) noexcept;

	// Overload for easier use with symbols provided by INCBIN.
	//
	// NOLINTNEXTLINE(*-c-arrays)
	Unpacker(const unsigned char (&bin)[], size_t binSize,
			 std::filesystem::path outdir = {}) noexcept
		: Unpacker({static_cast<std::decay_t<decltype(bin)>>(bin), binSize},
				   std::move(outdir)) {}

	Unpacker(const Unpacker&) = delete;
	Unpacker(Unpacker&&) = default;

	Unpacker& operator=(const Unpacker&) = delete;
	Unpacker& operator=(Unpacker&&) = default;

	// Recursively removes the output directory.
	~Unpacker() noexcept;

	[[nodiscard]] std::filesystem::path getOutDir() const noexcept {
		return outdir_;
	}
	[[nodiscard]] const PathVector& getFiles() const noexcept { return files_; }
	void setCleanup(bool cleanup) noexcept { cleanup_ = cleanup; }
};

}  // namespace embed
}  // namespace beholder

#endif	// BEHOLDER_EMBED_UNPACKER_H
