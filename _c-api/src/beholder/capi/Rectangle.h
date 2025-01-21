// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

// A stupid rectangle class.

#ifndef BEHOLDER_CAPI_RECTANGLE_H
#define BEHOLDER_CAPI_RECTANGLE_H

#ifdef __cplusplus
#include "beholder/capi/Wrapper.h"
#endif

#ifdef __cplusplus
namespace beholder {
namespace capi {
extern "C" {
#endif

typedef struct {  // NOLINT(modernize-use-using): C-API, so no 'using'
	int left;
	int top;
	int right;
	int bottom;
} Rectangle;

#ifdef __cplusplus
}  // extern "C"

namespace detail {
struct RectangleCtor {
	capi::Rectangle operator()() { return capi::Rectangle{0, 0, 0, 0}; }
};
}  // namespace detail
}  // namespace capi

// The actual class we use throughout the library.
using Rectangle = capi::Wrapper<capi::Rectangle, capi::detail::RectangleCtor>;

}  // namespace beholder

#endif	// __cplusplus

#endif	// BEHOLDER_CAPI_RECTANGLE_H
