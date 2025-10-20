// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

// TODO: remove at some point because we're probably never going to use this

#ifndef BEHOLDER_EMBED_TMPFS_H
#define BEHOLDER_EMBED_TMPFS_H

#include <cstddef>
#include <filesystem>

namespace beholder {
namespace embed {

// mountTmpfs creates a new tmpfs mount of size fsSize (in bytes) and returns
// the mountpoint path on success.
// If there is an error the returned path will be empty.
//
// NOTE: this works only on unix (probably just linux)
[[nodiscard]] std::filesystem::path mountTmpfs(size_t fsSize);

// umountTmpfs unmounts a tmpfs mount at mountpoint.
// Returns true on success, and false if there is an error.
//
// NOTE: this works only on unix (probably just linux)
[[nodiscard]] bool umountTmpfs(const std::filesystem::path& mountpoint);

}  // namespace embed
}  // namespace beholder

#endif	// BEHOLDER_EMBED_TMPFS_H
