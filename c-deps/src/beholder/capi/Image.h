// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

// A stupid image class.

#ifndef BEHOLDER_CAPI_IMAGE_H
#define BEHOLDER_CAPI_IMAGE_H

#ifdef __cplusplus
#include <cstddef>
#include <cstdint>

#include "beholder/capi/Wrapper.h"

namespace beholder {
namespace capi {
extern "C" {
#else
#include <stddef.h>
#include <stdint.h>
#endif	// __cplusplus

typedef struct {  // NOLINT(modernize-use-using): C-API, so no 'using'
	// Image ID.
	size_t id;
	// Number of image rows.
	int rows;
	// Number of image columns.
	int cols;
	// FIXME: this is pretty horrible, however simple values make
	// life easier from the Go side
	int64_t pixelType;
	// FIXME: this should be something better than a raw pointer,
	// however we can't do weak/shared pointers since Go will probably
	// manage this memory, so should think of something, because it
	// will cause issues
	// TODO: should have the buffer size
	void* buffer;
	size_t step;
	// Number of bits to store a pixel.
	// Cameras can return packed pixel types, so bytes are inappropriate.
	size_t bitsPerPixel;
} Image;

#ifdef __cplusplus
}  // extern "C"

namespace detail {
struct ImageCtor {
	capi::Image operator()() {
		return capi::Image{0UL, 0, 0, 0, nullptr, 0UL, 0UL};
	}
};
}  // namespace detail
}  // namespace capi

// The actual class we use throughout the library.
using Image = capi::Wrapper<capi::Image, capi::detail::ImageCtor>;

}  // namespace beholder

#endif	// __cplusplus

#endif	// BEHOLDER_CAPI_IMAGE_H
