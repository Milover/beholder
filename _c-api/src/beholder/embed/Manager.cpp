// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include "beholder/embed/Manager.h"

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <system_error>
#include <utility>

#include "beholder/embed/Embed.h"
#include "beholder/util/Errors.h"

namespace fs = std::filesystem;

namespace beholder {
namespace embed {

Manager::Manager(ConstByteSpan bin, fs::path outdir) noexcept
	: outdir_{std::move(outdir)} {
	if (outdir_.empty()) {
		// FIXME: replace std::tmpnam with something more appropriate, we're
		// using it just because it's portable (generate a UUID or something)
		outdir_ = fs::temp_directory_path() / std::tmpnam(nullptr);
	}
	// create tmpdir if necessary
	std::error_code err{};
	fs::create_directories(outdir_, err);
	if (err) {
		err::printErr(err);
		std::exit(EXIT_FAILURE);
	}
	// un-gz
	ByteVector decomp{decompressGzip(bin)};
	if (decomp.empty()) {
		std::exit(EXIT_FAILURE);
	}
	// un-tar (does the output)
	unarchiveTar(decomp, outdir_);
}

Manager::~Manager() noexcept {
	std::error_code err{};
	fs::remove_all(outdir_, err);
	if (err) {
		err::printErr(err);	 // FIXME: this can throw
	}
}

}  // namespace embed
}  // namespace beholder
