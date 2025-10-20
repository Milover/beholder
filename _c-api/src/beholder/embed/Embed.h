// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BEHOLDER_EMBED_EMBED_H
#define BEHOLDER_EMBED_EMBED_H

#include <span>
#include <vector>

namespace beholder {
namespace embed {

using ByteVector = std::vector<unsigned char>;
using CByteSpan = std::span<const unsigned char>;

// Uncompress gzip/libz format.
// If there is an error, the returned vector will be empty.
//
// TODO: we should return a result wrapping the value and an error value.
[[nodiscard]] ByteVector decompressGzip(CByteSpan data);

}  // namespace embed
}  // namespace beholder

#endif	// BEHOLDER_EMBED_EMBED_H
