// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

// Neural network implementation classes.

#ifndef BEHOLDER_NEURAL_INTERNAL_OBJ_DETECTOR_IMPL_H
#define BEHOLDER_NEURAL_INTERNAL_OBJ_DETECTOR_IMPL_H

#include <array>
#include <cassert>
#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/dnn/dnn.hpp>
#include <vector>

namespace beholder {
namespace internal {

// ObjDetectorNet is a simple struct which contains the neural network and
// parameters needed for converting and image to a blob.
//
// We use it so that we don't have to expose OpenCV headers in ObjDetector.h.
//
// We could have forward declared the two OpenCV classes, but they are defined
// in an inline namespace which is defined through a macro (CV__DNN_INLINE_NS),
// so if we we're to forward declare them, we would introduce an '#include'
// ordering dependency in ObjDetector.cpp.
// The namespace-macro would be 'forward-defined' in ObjDetector.h, and then
// we would rely on OpenCV headers being #included before ObjDetector.h
// in ObjDetector.cpp so that the namespace-macro is properly defined so that
// the forward declarations can work.
// This way is more straightforward even though it seems kinda dumb.
class ObjDetectorImpl {
private:
	using Net = cv::dnn::Net;
	using Params = cv::dnn::Image2BlobParams;

	cv::Mat blob_;					  // blob passed to the network
	std::unique_ptr<Net> net_;		  // the underlying neural network
	std::unique_ptr<Params> params_;  // conversion params

public:
	bool makeNet(const std::filesystem::path& model, cv::dnn::Backend b,
				 cv::dnn::Target t) {
		// XXX: no checks, we assume that it's been checked and is correct; yolo
		// TODO: we could supply a buffer and read from memory
		// TODO: we should also probably restrict support to ONNX files only,
		// because they seem to cause issues least frequently.
		net_ = std::make_unique<Net>(cv::dnn::readNet(model));
		if (!net_ || net_->empty()) {
			return false;
		}
		net_->setPreferableBackend(b);
		net_->setPreferableTarget(t);
		return true;
	}

	void makeParams(const cv::Scalar& scale, const cv::Size& size,
					const cv::Scalar& mean, bool swapRB,
					cv::dnn::ImagePaddingMode pm, const cv::Scalar& padValue) {
		params_ =
			std::make_unique<Params>(scale, size, mean, swapRB, CV_32F,
									 cv::dnn::DNN_LAYOUT_NCHW, pm, padValue);
	}

	[[nodiscard]] bool empty() const {
		return static_cast<bool>(net_) && net_->empty();
	}

	void infer(std::vector<cv::Mat>& outs) {
		assert(static_cast<bool>(net_));

		net_->forward(outs, net_->getUnconnectedOutLayersNames());
	}

	void setInput(const cv::Mat& img) {
		assert(static_cast<bool>(net_));
		assert(static_cast<bool>(params_));

		cv::dnn::blobFromImageWithParams(img, blob_, *params_);
		net_->setInput(blob_);
	}

	// TODO: will probably have to re-implement at some point because we
	// would like this to work for RotatedRects as well.
	// See TextDetectionModel_EAST_impl::detectTextRectangles for an example
	// implamentation.
	void
	transferBoxes(std::vector<cv::Rect>& boxes, const cv::Size& imgSize) const {
		assert(static_cast<bool>(params_));

		if (!boxes.empty()) {
			params_->blobRectsToImageRects(boxes, boxes, imgSize);
		}
	}
};

// Temporaries used during ObjDetector::detect and ObjDetector::extract.
class ObjDetectorBuffers {
public:
	std::vector<cv::Mat> outs;	// forward results
	// XXX: could also use RotatedRects
	std::vector<cv::Rect> tBoxes;	  // unfiltered blob boxes
	std::vector<double> tAngles;	  // box rotation angles
	std::vector<int> tClassIDs;		  // unfiltered class IDs
	std::vector<float> tConfidences;  // unfiltered confidences
	std::vector<int> tNMSIDs;		  // IDs used during NMS filtering

	// Clear buffers, but keep allocated memory.
	void clear() {
		outs.clear();
		tBoxes.clear();
		tAngles.clear();
		tClassIDs.clear();
		tConfidences.clear();
		tNMSIDs.clear();
	}
};

}  // namespace internal
}  // namespace beholder

#endif	// BEHOLDER_NEURAL_INTERNAL_OBJ_DETECTOR_IMPL_H
