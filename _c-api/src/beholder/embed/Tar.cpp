// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include "beholder/embed/Tar.h"

#include <cstdint>
#include <ostream>
#include <span>
#include <type_traits>

#include "beholder/util/Constants.h"
#include "beholder/util/Utility.h"

namespace csc = beholder::cst::charconv;

namespace beholder {
namespace embed {
namespace tar {

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

// NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
bool checksum(ConstByteSpan hdr) {
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
	std::span<const char> chksum{
		reinterpret_cast<const tar::Header*>(hdr.data())->chksum};

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
// NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

}  // namespace tar
}  // namespace embed
}  // namespace beholder
