// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BEHOLDER_EMBED_TAR_H
#define BEHOLDER_EMBED_TAR_H

#include <cstddef>

namespace beholder {
namespace embed {
namespace tar {

constexpr size_t blockSize{512};

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

}  // namespace tar
}  // namespace embed
}  // namespace beholder

#endif	// BEHOLDER_EMBED_TAR_H
