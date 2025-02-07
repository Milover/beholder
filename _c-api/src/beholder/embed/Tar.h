// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BEHOLDER_EMBED_TAR_H
#define BEHOLDER_EMBED_TAR_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <ostream>
#include <span>
#include <type_traits>

using ByteSpan = std::span<unsigned char>;

namespace beholder {
namespace embed {
namespace tar {

namespace detail {

// placeholderChksum computes (at compile-time) the placeholder value for
// the 'chksum' field in the tar header.
//
// A tar checksum is computed by summing all of the bytes in the header
// (sometimes it's signed, sometimes unsigned), but while computing,
// the 'chksum' field is assumed to be ASCII spaces (0x20).
template<typename T, std::enable_if_t<std::is_same_v<T, int64_t> ||
										  std::is_same_v<T, uint64_t>,
									  bool> = true>
consteval T placeholderChksum() noexcept {
	constexpr std::array plh{'\x20', '\x20', '\x20', '\x20',
							 '\x20', '\x20', '\x20', '\x20'};
	return std::accumulate(plh.begin(), plh.end(), 0, [](auto&& acc, auto i) {
		return acc + static_cast<T>(i);
	});
}

}  // namespace detail

constexpr size_t blockSize{512};

template<typename T>
constexpr T placeholderChecksum_v = detail::placeholderChksum<T>();

// NOLINTBEGIN(*-c-arrays, *-magic-numbers)

// The UStar header.
//
// https://www.gnu.org/software/tar/manual/html_node/Standard.html
struct Header {
	char name[100];		 // file name
	char mode[8];		 // file mode (octal)
	char uid[8];		 // UID
	char gid[8];		 // GID
	char size[12];		 // file size in bytes (octal)
	char mtime[12];		 // modification time (octal)
	char chksum[8];		 // checksum for header record
	char typeflag;		 // file type
	char linkname[100];	 // name of linked file
	char magic[6];		 // UStar indicator "ustar"
	char version[2];	 // UStar version
	char uname[32];		 // owner user name
	char gname[32];		 // owner group name
	char devmajor[8];	 // device major number
	char devminor[8];	 // device minor number
	char prefix[155];	 // filename prefix
	char padding[12];
};

static_assert(sizeof(Header) == blockSize);	 // sanity check

enum class FileType : char {
	Reg = '0',		// regular file
	RegA = '\0',	// regular file
	Link = '1',		// hard link
	Symlink = '2',	// symbolic link
	Char = '3',		// character device node
	Block = '4',	// block device node
	Dir = '5',		// directory
	Fifo = '6',		// FIFO node
	Cont = '7',		// reserved
	XHeader = 'x',	// extended header referring to the next file in the archive
	XGlobalHeader = 'g',  // global extended header
	GNUSparse = 'S',	  // sparse file in the GNU format
	GNULongName = 'L',	  // long path name meta file in the GNU format
	GNULongLink = 'K',	  // long link name meta file in the GNU format
};

// NOLINTEND(*-c-arrays, *-magic-numbers)

// checksum checks the tar header (signed and unsigned) checksum.
bool checksum(ByteSpan hdr);

// global stream output operator
std::ostream& operator<<(std::ostream& os, FileType t);

}  // namespace tar
}  // namespace embed
}  // namespace beholder

#endif	// BEHOLDER_EMBED_TAR_H
