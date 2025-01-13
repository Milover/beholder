// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#include "beholder/neural/PARSeqDetector.h"

#include <cmath>
#include <opencv2/core.hpp>
#include <opencv2/core/fast_math.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/dnn/dnn.hpp>
#include <utility>

#include "beholder/neural/internal/ObjDetectorBuffers.h"
#include "beholder/util/Constants.h"

namespace beholder {

void PARSeqDetector::extract() {
	if (buf_->outs.size() != 1) {
		return;
	}
	// for the pretrained model, the output should be [1, 26, 95],
	// 25+1 positions (results) for 1+94 chars (the first char is a blank)
	const int chsetLen{static_cast<int>(charset.size())};
	cv::Mat out{buf_->outs[0]};
	if (out.size[1] != nPos || out.size[2] != chsetLen + 1 ||
		out.type() != CV_32FC1) {
		return;
	}

	Result r{};
	r.confidence = 1.0;

	// TODO: pull this out, we should be able to use different decoders
	for (auto pos{0}; pos < nPos; ++pos) {
		const float* pred{out.ptr<float>(0, pos)};

		int maxLoc{0};	// index of the max score
		// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
		double maxScore{pred[0]};  // (initial) max score value
		double softMaxD{0.0};	   // softmax denominator (sum of exp values)
		for (auto id{0}; id < chsetLen; ++id) {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
			const double score{static_cast<double>(pred[id])};
			if (maxScore < score) {
				maxScore = score;
				maxLoc = id;
			}
			softMaxD += std::exp(score);
		}
		if (maxLoc > 0) {
			r.text += charset.at(maxLoc - 1);
			r.confidence *= std::exp(maxScore) / softMaxD;
		} else {
			// include the EOS probability as well
			r.confidence *= std::exp(maxScore) / softMaxD;
			break;
		}
	}
	if (!r.text.empty()) {
		res_.emplace_back(std::move(r));
	}
}

PARSeqDetector::PARSeqDetector() {
	// no padding or cropping, the input image should be just the
	// word/character sequence which is to be evaluated/recognized
	resizeMode_ = Base::ResizeMode::ResizeRaw;

	// For more info, see:
	// https://github.com/baudm/parseq/blob/1902db043c029a7e03a3818c616c06600af574be/strhub/data/module.py#L69
	// NOLINTBEGIN(*-magic-numbers)
	scale = Base::Vec3<>{0.5 / cst::max8bit, 0.5 / cst::max8bit,
						 0.5 / cst::max8bit};
	mean = Base::Vec3<>{0.5 * cst::max8bit, 0.5 * cst::max8bit,
						0.5 * cst::max8bit};
	// NOLINTEND(*-magic-numbers)
}

}  // namespace beholder
