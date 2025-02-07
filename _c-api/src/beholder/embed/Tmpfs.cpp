// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include "beholder/embed/Tmpfs.h"

#include <sys/mount.h>

#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <string>
#include <system_error>

#include "beholder/util/Errors.h"

namespace fs = std::filesystem;

namespace beholder {
namespace embed {

constexpr static const char* fstypeTmpfs{"tmpfs"};

fs::path mountTmpfs(size_t fsSize) {
	// FIXME: replace std::tmpnam with something more appropriate, we're
	// using it just because it's portable (generate a UUID or something)
	fs::path mnt{fs::temp_directory_path() / std::tmpnam(nullptr)};

	// we could also set uid/gid/mode; see man tmpfs(5) for more options
	const std::string opts{"size=" + std::to_string(fsSize)};
	const int ec{mount(fstypeTmpfs, mnt.c_str(), fstypeTmpfs, 0, opts.c_str())};
	if (ec != 0) {
		std::error_code err{err::fromErrno(errno)};
		if (err) {
			err::printErr(err);
			// cleanup
			fs::remove(mnt, err);
			if (err) {
				err::printErr(err);
			}
			return fs::path{};
		}
	}
	return mnt;
}

bool umountTmpfs(const fs::path& mountpoint) {
	const int ec{umount(mountpoint.c_str())};
	if (ec != 0) {
		const std::error_code err{err::fromErrno(errno)};
		if (err) {
			err::printErr(err);
			return false;
		}
	}
	return true;
}

}  // namespace embed
}  // namespace beholder
