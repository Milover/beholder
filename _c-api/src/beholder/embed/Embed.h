// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BEHOLDER_EMBED_EMBED_H
#define BEHOLDER_EMBED_EMBED_H

#include <cstddef>
#include <filesystem>
#include <span>
#include <vector>

namespace beholder {
namespace embed {

using ByteVector = std::vector<unsigned char>;
using ByteSpan = std::span<unsigned char>;

// TODO: we should return a result wrapping the value and an error value.
[[nodiscard]] ByteVector decompressGzip(ByteSpan data);

// unarchiveTar extracts a tar archive using 'root' as the output directory, or
// into PWD if 'root' is not supplied.
void unarchiveTar(ByteSpan data, const std::filesystem::path& root = {});

}  // namespace embed
}  // namespace beholder

#endif	// BEHOLDER_EMBED_EMBED_H
