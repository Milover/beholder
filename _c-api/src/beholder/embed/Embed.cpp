// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include "beholder/embed/Embed.h"

#include <zlib.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "beholder/util/ScopeGuard.h"

namespace beholder {
namespace embed {

constexpr static int zlibinflateFlag{15};  // inflate windowBits
constexpr static int zlibAutoFmtFlag{32};  // auto-fmt windowBits

ByteVector decompressGzip(CByteSpan data) {
	z_stream zs{};
	zs.next_in = data.data();
	zs.avail_in = data.size();

	int ok{inflateInit2(&zs, zlibinflateFlag + zlibAutoFmtFlag)};
	// TODO: should return a result wrapper
	if (ok != Z_OK) {
		std::cerr << "zlib error (" << ok << "): " << zs.msg << std::endl;
		return ByteVector{};
	}
	const ScopeGuard guard{[&]() noexcept { inflateEnd(&zs); }};

	ByteVector out(data.size());  // we'll write at least data.size bytes
	while (ok != Z_STREAM_END) {
		zs.next_out = out.data() + zs.total_out;  // NOLINT(cppcoreguidelines-*)
		zs.avail_out = out.size() - zs.total_out;

		ok = inflate(&zs, Z_NO_FLUSH);
		// TODO: should return a result wrapper
		if (ok != Z_OK && ok != Z_STREAM_END) {
			std::cerr << "zlib error (" << ok << "): " << zs.msg << std::endl;
			out.clear();
			return ByteVector{};
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

}  // namespace embed
}  // namespace beholder
