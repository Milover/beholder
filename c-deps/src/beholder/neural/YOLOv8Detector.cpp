// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include "beholder/neural/YOLOv8Detector.h"

#include <opencv2/core.hpp>
#include <opencv2/core/fast_math.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/dnn/dnn.hpp>
#include <utility>

#include "beholder/neural/internal/ObjDetectorImpl.h"
#include "beholder/util/Constants.h"

namespace beholder {

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
void YOLOv8Detector::extract() {
	if (buf_->outs.size() != 1) {
		return;
	}

	cv::Mat out{buf_->outs[0]};
	if (out.dims != 3 ||
		out.size[2] < 5)  // NOLINT: should have 4 coords and at least 1 class
	{
		return;
	}
	cv::transposeND(out, {0, 2, 1}, out);
	out = out.reshape(1, out.size[1]);	// [1, 8400, 85] -> [8400, 85]
	cv::Mat scores{};
	for (auto i{0}; i < out.rows; ++i) {
		double conf{};
		cv::Point maxLoc{};
		scores = out.row(i).colRange(
			4, out.cols);  // just a 'view', so should be efficient
		cv::minMaxLoc(scores, nullptr, &conf, nullptr, &maxLoc);

		if (conf < confidenceThreshold) {
			continue;
		}

		// get bbox coords; [xCenter, yCenter, width, height]
		float* det{out.ptr<float>(i)};
		buf_->tBoxes.emplace_back(cvFloor(det[0] - det[2] / 2),
								  cvFloor(det[1] - det[3] / 2), cvFloor(det[2]),
								  cvFloor(det[3]));
		buf_->tClassIDs.emplace_back(maxLoc.x);
		buf_->tConfidences.emplace_back(static_cast<float>(conf));
	}
}
// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

void YOLOv8Detector::store() {
	cv::dnn::NMSBoxes(buf_->tBoxes, buf_->tConfidences, confidenceThreshold,
					  nmsThreshold, buf_->tNMSIDs);

	res_.reserve(buf_->tNMSIDs.size());
	for (auto i{0UL}; i < buf_->tNMSIDs.size(); ++i) {
		Result r{};

		auto id{buf_->tNMSIDs[i]};	// use NMS filtered IDs to select results
		const cv::Rect& b{buf_->tBoxes[id]};
		auto classID{buf_->tClassIDs[id]};

		r.box = Rectangle{b.x, b.y, b.x + b.width, b.y + b.height},
		r.confidence = static_cast<double>(buf_->tConfidences[id]);
		r.text = classes.size() > static_cast<size_t>(classID)
					 ? classes[classID]
					 : std::to_string(classID);

		res_.emplace_back(std::move(r));
	}
}

YOLOv8Detector::YOLOv8Detector() {
	scale = Base::Vec3<>{1.0 / cst::max8bit, 1.0 / cst::max8bit,
						 1.0 / cst::max8bit};
}

}  // namespace beholder
