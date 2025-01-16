// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include "beholder/neural/EASTDetector.h"

#include <array>
#include <cmath>
#include <opencv2/core.hpp>
#include <opencv2/core/fast_math.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/dnn/dnn.hpp>

#include "beholder/neural/internal/ObjDetectorImpl.h"

namespace beholder {

// NOLINTBEGIN(*-magic-numbers, cppcoreguidelines-pro-bounds-pointer-arithmetic)
void EASTDetector::extract() {
	if (buf_->outs.size() != 2) {
		return;
	}
	//CV_CheckEQ(buf_->outs.size(), (size_t)2, "");
	cv::Mat geometry{buf_->outs[0]};
	cv::Mat confsMap{buf_->outs[1]};

	if (confsMap.dims != 4 || geometry.dims != 4 || confsMap.size[0] != 1 ||
		geometry.size[0] != 1 || confsMap.size[1] != 1 ||
		geometry.size[1] != 5 || confsMap.size[2] != geometry.size[2] ||
		confsMap.size[3] != geometry.size[3] || confsMap.type() != CV_32FC1 ||
		geometry.type() != CV_32FC1) {
		return;
	}

	const int height{confsMap.size[2]};
	const int width{confsMap.size[3]};
	for (auto y{0}; y < height; ++y) {
		const float* confs = confsMap.ptr<float>(0, 0, y);
		const float* x0s = geometry.ptr<float>(0, 0, y);
		const float* x1s = geometry.ptr<float>(0, 1, y);
		const float* x2s = geometry.ptr<float>(0, 2, y);
		const float* x3s = geometry.ptr<float>(0, 3, y);
		const float* angles = geometry.ptr<float>(0, 4, y);
		for (auto x{0}; x < width; ++x) {
			const float conf{confs[x]};
			if (conf < confidenceThreshold) {
				continue;
			}

			const float offsetX{static_cast<float>(x) * 4.0F};
			const float offsetY{static_cast<float>(y) * 4.0F};
			const float angle{angles[x]};
			const float cosA{std::cos(angle)};
			const float sinA{std::sin(angle)};
			const float h{x0s[x] + x2s[x]};
			const float w{x1s[x] + x3s[x]};

			const cv::Point2f offset{offsetX + cosA * x1s[x] + sinA * x2s[x],
									 offsetY - sinA * x1s[x] + cosA * x2s[x]};
			const cv::Point2f p1{cv::Point2f{-sinA * h, -cosA * h} + offset};
			const cv::Point2f p3{cv::Point2f{-cosA * w, sinA * w} + offset};

			buf_->tBoxes.emplace_back(
				cv::RotatedRect{0.5F * (p1 + p3), cv::Size2f(w, h), 0.0}
					.boundingRect());
			buf_->tAngles.emplace_back(static_cast<double>(-angle) * 180.0 /
									   CV_PI);
			buf_->tConfidences.emplace_back(conf);
		}
	}
}
// NOLINTEND(*-magic-numbers, cppcoreguidelines-pro-bounds-pointer-arithmetic)

void EASTDetector::store() {
	cv::dnn::NMSBoxes(buf_->tBoxes, buf_->tConfidences, confidenceThreshold,
					  nmsThreshold, buf_->tNMSIDs);

	res_.reserve(buf_->tNMSIDs.size());
	for (auto i{0UL}; i < buf_->tNMSIDs.size(); ++i) {
		Result r{};

		auto id{buf_->tNMSIDs[i]};	// use NMS filtered IDs to select results
		const cv::Rect& b{buf_->tBoxes[id]};

		r.boxRotAngle = buf_->tAngles[id];
		r.box = Rectangle{b.x, b.y, b.x + b.width, b.y + b.height},
		r.confidence = static_cast<double>(buf_->tConfidences[id]);

		res_.emplace_back(std::move(r));
	}
}

EASTDetector::EASTDetector() {
	// For more info, see:
	// https://docs.opencv.org/4.10.0/d4/d43/tutorial_dnn_text_spotting.html
	// NOLINTNEXTLINE(*-magic-numbers)
	mean = Base::Vec3<>{123.68, 116.78, 103.94};
}

}  // namespace beholder
