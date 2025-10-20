// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include "beholder/capi/Result.h"

namespace beholder {
namespace capi {
extern "C" {

void Result_Delete(Result* r) {
	if (static_cast<bool>(r)) {
		delete[] r->text;
	}
}

}  // namespace capi
}  // namespace capi
}  // namespace beholder
