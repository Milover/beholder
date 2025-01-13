// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

// A stupid class for holding various image detection results

#ifndef BEHOLDER_CAPI_RESULT_H
#define BEHOLDER_CAPI_RESULT_H

#ifdef __cplusplus
#include <cstddef>
#include <cstring>
#include <string>
#include <type_traits>

#include "beholder/util/Traits.h"
#else
#include <stddef.h>
#endif

#include "beholder/capi/Rectangle.h"

#ifdef __cplusplus
namespace beholder {
namespace capi {
extern "C" {
#endif

// We use this guy only from Go because the char* is cancer.
//
// TODO: should we hold an image or a reference to an image?
typedef struct {  // NOLINT(modernize-use-using): C-API, so no 'using'
	// Recognized/detected text.
	char* text;
	// Bounding boxes detected by algorithms and/or NNs.
	Rectangle box;
	// Bounding box rotation angle, with respect to the original image.
	double boxRotAngle;
	// Confidence of the result.
	double confidence;

} Result;

// Free memory held by r.
void Result_Delete(Result* r);

#ifdef __cplusplus
}  // extern "C"
}  // namespace capi

// We always use this guy where we can.
//
// FIXME: we don't like how this is implemented.
class Result {
public:
	// Recognized/detected text.
	std::string text;
	// Bounding boxes detected by algorithms and/or NNs.
	Rectangle box;
	// Bounding box rotation angle, with respect to the original image.
	double boxRotAngle{};
	// Confidence of the result.
	double confidence{};

	// Default constructor.
	Result() = default;

	// Construct by forwarding components
	template<typename Txt, typename Box, typename Ang, typename Conf>
	Result(Txt&& t, Box&& b, Ang&& rot, Conf&& c)
		: text{std::forward<Txt>(t)},
		  box{std::forward<Box>(b)},
		  boxRotAngle{std::forward<Ang>(rot)},
		  confidence{std::forward<Conf>(c)} {}

	// Construct by copying a capi::Result
	explicit Result(const capi::Result& r)
		: text{r.text},
		  box{r.box},
		  boxRotAngle{r.boxRotAngle},
		  confidence{r.confidence} {}

	Result(const Result&) = default;
	Result(Result&&) = default;

	~Result() = default;

	Result& operator=(const Result&) = default;
	Result& operator=(Result&&) = default;

	// Return a capi::Result copy
	[[nodiscard]] capi::Result toC() const {
		char* ch{new char[text.size() + 1]};
		std::strncpy(ch, text.c_str(), text.size());
		return capi::Result{ch, box.cRef(), boxRotAngle, confidence};
	}
};

}  // namespace beholder

#endif	// __cplusplus

#endif	// BEHOLDER_CAPI_RESULT_H
