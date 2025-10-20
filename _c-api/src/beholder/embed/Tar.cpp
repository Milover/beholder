// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include "beholder/embed/Tar.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <span>
#include <system_error>
#include <type_traits>
#include <utility>

#include "beholder/util/Constants.h"
#include "beholder/util/Enums.h"
#include "beholder/util/Errors.h"
#include "beholder/util/Utility.h"

namespace fs = std::filesystem;
namespace csc = beholder::cst::charconv;

namespace beholder {
namespace embed {
namespace tar {

using CCharSpan = std::span<const char>;

std::ostream& operator<<(std::ostream& os, FileType t) {
	switch (t) {
		case FileType::Reg: {
			[[fallthrough]];
			case FileType::RegA: {
				os << "regular";
				break;
			}
			case FileType::Link: {
				os << "link";
				break;
			}
			case FileType::Symlink: {
				os << "symlink";
				break;
			}
			case FileType::Char: {
				os << "character";
				break;
			}
			case FileType::Block: {
				os << "block";
				break;
			}
			case FileType::Dir: {
				os << "directory";
				break;
			}
			case FileType::Fifo: {
				os << "FIFO";
				break;
			}
			case FileType::Cont: {
				os << "reserved";
				break;
			}
			case FileType::XHeader: {
				os << "x-header";
				break;
			}
			case FileType::XGlobalHeader: {
				os << "x-global-header";
				break;
			}
			case FileType::GNUSparse: {
				os << "GNU-sparse";
				break;
			}
			case FileType::GNULongName: {
				os << "GNU-long-path";
				break;
			}
			case FileType::GNULongLink: {
				os << "GNU-long-link";
				break;
			}
			default: {
				os << "unknown";
				break;
			}
		}
	}
	return os;
}

bool checksum(CByteSpan hdr) {
	// the checksum can be either signed or unsigned, either which is fine
	int64_t sSum{};
	uint64_t uSum{};

	// the tar checksum is a simple sum of all the header bytes
	for (auto i{0UL}; i < hdr.size(); ++i) {
		sSum += static_cast<int64_t>(hdr[i]);
		uSum += static_cast<uint64_t>(hdr[i]);
	}
	// correct the checksum contribution
	// NOTE: see comments for tar::detail::placeholderChksum
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
	CCharSpan chksum{reinterpret_cast<const tar::Header*>(hdr.data())->chksum};

	auto correctSum = [&chksum](auto& sum) {
		using Int = std::decay_t<decltype(sum)>;
		sum -= std::accumulate(
			chksum.begin(), chksum.end(), 0,
			[](auto&& acc, auto i) { return acc + static_cast<Int>(i); });
		sum += tar::placeholderChecksum_v<Int>;
	};
	correctSum(sSum);
	correctSum(uSum);

	const int64_t sChksum{toDecimal<int64_t, csc::base8>(chksum)};
	const uint64_t uChksum{toDecimal<uint64_t, csc::base8>(chksum)};

	return sSum == sChksum || uSum == uChksum;
}

PathVector unarchive(CByteSpan data, const fs::path& root) {
	PathVector paths;
	paths.reserve(5);  // NOLINT: guesstimate

	size_t offset{};
	while (offset + BlockSize <= data.size()) {
		// check if this header block is zeroed out (end-of-archive marker)
		if (std::ranges::all_of(data.subspan(offset, BlockSize),
								[](auto&& v) { return v == '\0'; })) {
			break;
		}
		if (!checksum(data.subspan(offset, BlockSize))) {
			std::cerr << "warning: checksum mismatch" << '\n';
			// TODO: we should skip until the next valid header or something
		}
		const Header* hdr{
			reinterpret_cast<const Header*>(data.data() + offset)};	 // NOLINT
		const size_t fileSize{toDecimal<size_t, csc::base8>(hdr->size)};

		fs::path filename = fs::path{hdr->name};
		if (hdr->prefix[0] != '\0') {
			filename = fs::path{hdr->prefix} / filename;
		}
		fs::path outPath{root / filename};

		// handle files
		std::error_code err{};
		const FileType flag{enums::from<FileType>(hdr->typeflag)};
		if (flag == FileType::Dir) {
			err = handleDirectory(std::move(outPath), paths);
		} else if (flag == FileType::Reg || flag == FileType::RegA) {
			const CByteSpan content{data.subspan(offset + BlockSize, fileSize)};
			err = handleFile(std::move(outPath), hdr, content, paths);
		} else if (flag == FileType::Symlink) {
			err = handleSymlink(std::move(outPath), hdr, paths);
		} else {
			std::cerr << "skipping unsupported file type (" << flag
					  << "): " << outPath;
		}
		if (err) {
			err::printErr(err);
		}
		// move offset (header block + file data rounded up to next block)
		size_t totalSize = BlockSize;
		if (fileSize > 0) {
			// ceil avoiding overflow
			const size_t fileBlocks{1 + ((fileSize - 1) / BlockSize)};
			totalSize += fileBlocks * BlockSize;
		}
		offset += totalSize;
	}
	return paths;
}

std::error_code handleDirectory(fs::path&& p, PathVector& store) noexcept {
	std::error_code err{};
	fs::create_directories(p, err);
	if (err) {
		return err;
	}
	store.emplace_back(std::move(p));
	return err;
}

std::error_code handleFile(fs::path&& p, const Header* hdr, CByteSpan content,
						   PathVector& store) noexcept {
	std::error_code err{};
	fs::create_directories(p.parent_path(), err);
	if (err) {
		return err;
	}
	std::ofstream ofs{p, std::ios::binary | std::ios::trunc};
	if (!ofs) {
		std::cerr << "io error: failed to open: " << p << std::endl;
		return std::make_error_code(std::io_errc::stream);
	}
	// write only the actual file content (data is padded up to next block)
	ofs.write(reinterpret_cast<const char*>(content.data()),  // NOLINT
			  static_cast<std::streamsize>(content.size()));
	if (!ofs) {
		std::cerr << "io error: " << p << std::endl;
		return std::make_error_code(std::io_errc::stream);
	}
	ofs.close();
	// set permissions
	const fs::perms perms{
		enums::from<fs::perms>(toDecimal<size_t, csc::base8>(hdr->mode))};
	fs::permissions(p, perms, err);
	if (err) {
		return err;
	}
	store.emplace_back(std::move(p));
	return err;
}

std::error_code
handleSymlink(fs::path&& p, const Header* hdr, PathVector& store) noexcept {
	std::error_code err{};
	fs::create_directories(p.parent_path(), err);
	if (err) {
		return err;
	}
	// FIXME: when linking directories, we should use
	// create_directory_symlink, however, we don't need this atm
	fs::create_symlink(hdr->linkname, p, err);
	if (err) {
		return err;
	}
	store.emplace_back(std::move(p));
	return err;
}

}  // namespace tar
}  // namespace embed
}  // namespace beholder
