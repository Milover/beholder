// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include "beholder/embed/Unpacker.h"

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <system_error>
#include <utility>

#include "beholder/embed/Embed.h"
#include "beholder/embed/Tar.h"
#include "beholder/util/Errors.h"

namespace fs = std::filesystem;

namespace beholder {
namespace embed {

void Unpacker::unpack(CByteSpan bin) noexcept {
	if (outdir_.empty()) {
		// FIXME: replace std::tmpnam with something more appropriate, we're
		// using it just because it's portable (generate a UUID or something)
		outdir_ = fs::temp_directory_path() / std::tmpnam(nullptr);
	}
	std::error_code err{};
	fs::create_directories(outdir_, err);
	if (err) {
		err::printErr(err);
		std::exit(EXIT_FAILURE);
	}
	ByteVector decomp{decompressGzip(bin)};
	if (decomp.empty()) {
		std::exit(EXIT_FAILURE);
	}
	files_ = tar::unarchive(decomp, outdir_);
}

Unpacker::Unpacker(CByteSpan bin, fs::path outdir) noexcept
	: outdir_{std::move(outdir)} {
	unpack(bin);
}

Unpacker::~Unpacker() noexcept {
	if (!cleanup_) {
		return;
	}
	std::error_code err{};
	fs::remove_all(outdir_, err);
	if (err) {
		err::printErr(err);	 // FIXME: this can throw
	}
}

}  // namespace embed
}  // namespace beholder
