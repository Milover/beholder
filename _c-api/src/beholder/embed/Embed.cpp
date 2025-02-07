// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include "beholder/embed/Embed.h"

#include <zlib.h>

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <system_error>

#include "beholder/embed/Tar.h"
#include "beholder/util/Constants.h"
#include "beholder/util/Enums.h"
#include "beholder/util/Errors.h"
#include "beholder/util/ScopeGuard.h"
#include "beholder/util/Utility.h"

namespace fs = std::filesystem;
namespace csc = beholder::cst::charconv;

namespace beholder {
namespace embed {

constexpr static int zlibinflateFlag{15};  // inflate windowBits
constexpr static int zlibAutoFmtFlag{32};  // auto-fmt windowBits

ByteVector decompressGzip(ByteSpan data) {
	ByteVector out{};

	z_stream zs{};
	zs.next_in = data.data();
	zs.avail_in = data.size();

	int ok{inflateInit2(&zs, zlibinflateFlag + zlibAutoFmtFlag)};
	// TODO: should return a result wrapper
	if (ok != Z_OK) {
		std::cerr << "zlib error (" << ok << "): " << zs.msg << std::endl;
		return out;
	}
	const ScopeGuard guard{[&]() noexcept { inflateEnd(&zs); }};

	out.resize(data.size());  // we'll write at least data.size bytes
	while (ok != Z_STREAM_END) {
		zs.next_out = out.data() + zs.total_out;  // NOLINT(cppcoreguidelines-*)
		zs.avail_out = out.size() - zs.total_out;

		ok = inflate(&zs, Z_NO_FLUSH);
		// TODO: should return a result wrapper
		if (ok != Z_OK && ok != Z_STREAM_END) {
			std::cerr << "zlib error (" << ok << "): " << zs.msg << std::endl;
			return out;
		}
		// if we're not done, then we're full and should grow
		if (ok != Z_STREAM_END) {
			out.resize(2 * out.size());
		}
	}
	out.resize(zs.total_out);  // chop off the extra bits
	out.shrink_to_fit();

	return out;
}

// NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
void unarchiveTar(ByteSpan data, const fs::path& root) {
	size_t offset{};
	while (offset + tar::blockSize <= data.size()) {
		// check if this header block is zeroed out (end-of-archive marker)
		if (std::ranges::all_of(data.subspan(offset, tar::blockSize),
								[](auto&& v) { return v == '\0'; })) {
			break;
		}
		if (!tar::checksum(data.subspan(offset, tar::blockSize))) {
			std::cerr << "warning: checksum mismatch" << '\n';
			// TODO: we should skip until the next valid header or something
		}
		const tar::Header* header{
			reinterpret_cast<const tar::Header*>(data.data() + offset)};
		const size_t fileSize{toDecimal<size_t, csc::base8>(header->size)};

		fs::path filename = fs::path{header->name};
		if (header->prefix[0] != '\0') {
			filename = fs::path{header->prefix} / filename;
		}
		const fs::path outPath{root / filename};

		// handle files
		switch (auto flag{enums::from<tar::FileType>(header->typeflag)}; flag) {
			case tar::FileType::Dir: {
				fs::create_directories(outPath);
				break;
			}
			case tar::FileType::Reg:
				[[fallthrough]];
			case tar::FileType::RegA: {
				// ensure the parent directory exists
				fs::create_directories(fs::path(outPath).parent_path());
				std::ofstream ofs{outPath, std::ios::binary | std::ios::trunc};
				if (!ofs) {
					std::cerr << "io error: failed to open: " << outPath
							  << std::endl;
					break;
				}
				// the file data immediately follows the 512-byte header
				const ByteSpan content{
					data.subspan(offset + tar::blockSize, fileSize)};
				// write only the actual file content (data is padded up to next block)
				ofs.write(reinterpret_cast<const char*>(content.data()),
						  static_cast<std::streamsize>(content.size()));
				if (!ofs) {
					std::cerr << "io output error: " << outPath << std::endl;
					break;
				}
				ofs.close();
				// set permissions
				std::error_code err{};
				fs::permissions(
					outPath,
					enums::from<fs::perms>(
						toDecimal<size_t, csc::base8>(header->mode)),
					err);
				if (err) {
					err::printErr(err);
				}
				break;
			}
			case tar::FileType::Symlink: {
				std::error_code err{};
				fs::create_directories(fs::path(outPath).parent_path());
				// FIXME: when linking to directories, we should use
				// create_directory_symlink, however, we don't need this atm
				fs::create_symlink(header->linkname, outPath, err);
				if (err) {
					err::printErr(err);
				}
				break;
			}
			default: {
				std::cerr << "skipping unsupported file type (" << flag
						  << "): " << outPath;
				// TODO: unsupported file types are skipped
				break;
			}
		}
		// move offset (header block + file data rounded up to next block)
		size_t totalSize = tar::blockSize;
		if (fileSize > 0) {
			// ceil avoiding overflow
			const size_t fileBlocks{1 + ((fileSize - 1) / tar::blockSize)};
			totalSize += fileBlocks * tar::blockSize;
		}
		offset += totalSize;
	};
};
// NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

}  // namespace embed
}  // namespace beholder
