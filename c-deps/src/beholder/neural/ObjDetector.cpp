// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#include "beholder/neural/ObjDetector.h"

#include <filesystem>
#include <memory>
#include <opencv2/dnn/dnn.hpp>
#include <vector>

#include "beholder/capi/Image.h"
#include "beholder/image/Processor.h"
#include "beholder/neural/internal/ObjDetectorImpl.h"
#include "beholder/util/Enums.h"

namespace beholder {

namespace {
// Static checks which enforce compliance between our parameter types and
// the OpenCV supported backends.
using Bnd = NNBackend;
static_assert(Bnd::BackendDefault == cv::dnn::DNN_BACKEND_DEFAULT);
static_assert(Bnd::BackendHalide == cv::dnn::DNN_BACKEND_HALIDE);
static_assert(Bnd::BackendOpenVINO == cv::dnn::DNN_BACKEND_INFERENCE_ENGINE);
static_assert(Bnd::BackendOpenCV == cv::dnn::DNN_BACKEND_OPENCV);
static_assert(Bnd::BackendVulkan == cv::dnn::DNN_BACKEND_VKCOM);
static_assert(Bnd::BackendCUDA == cv::dnn::DNN_BACKEND_CUDA);
static_assert(Bnd::BackendWebNN == cv::dnn::DNN_BACKEND_WEBNN);
static_assert(Bnd::BackendTIMVX == cv::dnn::DNN_BACKEND_TIMVX);
static_assert(Bnd::BackendCANN == cv::dnn::DNN_BACKEND_CANN);

// Static checks which enforce compliance between our parameter types and
// the OpenCV supported targets.
using Tgt = NNTarget;
static_assert(Tgt::TargetCPU == cv::dnn::DNN_TARGET_CPU);
static_assert(Tgt::TargetOpenCL == cv::dnn::DNN_TARGET_OPENCL);
static_assert(Tgt::TargetOpenCLfp16 == cv::dnn::DNN_TARGET_OPENCL_FP16);
static_assert(Tgt::TargetMyriad == cv::dnn::DNN_TARGET_MYRIAD);
static_assert(Tgt::TargetVulkan == cv::dnn::DNN_TARGET_VULKAN);
static_assert(Tgt::TargetFPGA == cv::dnn::DNN_TARGET_FPGA);
static_assert(Tgt::TargetCUDA == cv::dnn::DNN_TARGET_CUDA);
static_assert(Tgt::TargetCUDAfp16 == cv::dnn::DNN_TARGET_CUDA_FP16);
static_assert(Tgt::TargetHDDL == cv::dnn::DNN_TARGET_HDDL);
static_assert(Tgt::TargetNPU == cv::dnn::DNN_TARGET_NPU);
static_assert(Tgt::TargetCPUfp16 == cv::dnn::DNN_TARGET_CPU_FP16);

// Static checks which enforce compliance between our parameter types and
// the OpenCV DNN module (blob) padding modes.
using Mod = ObjDetector::ResizeMode;
static_assert(Mod::ResizeRaw == cv::dnn::DNN_PMODE_NULL);
static_assert(Mod::ResizeCrop == cv::dnn::DNN_PMODE_CROP_CENTER);
static_assert(Mod::ResizeLetterbox == cv::dnn::DNN_PMODE_LETTERBOX);
}  // namespace

// NOLINTNEXTLINE(*-use-equals-default): incomplete type; must be defined here
ObjDetector::~ObjDetector(){};

void ObjDetector::clear() {
	if (buf_) {
		buf_->clear();
	}
	res_.clear();
}

bool ObjDetector::detect(const Image& raw) {
	clear();

	if (!impl_ || impl_->empty()) {
		return false;
	}
	auto img{rawToMatPtr(raw)};
	if (!img) {
		return false;
	}
	impl_->setInput(*img);
	// TODO: not sure how clear() and buf_->outs impact re-allocation,
	// should check at some point.
	// Ideally we would want to keep the memory allocated by
	// each of the buf_->outs, instead of throwing it away each time.
	// XXX: what were we talking about here?
	impl_->infer(buf_->outs);

	extract();
	impl_->transferBoxes(buf_->tBoxes, img->size());

	// store results
	store();

	return !res_.empty();
}

const std::vector<Result>& ObjDetector::getResults() const { return res_; }

bool ObjDetector::init() {
	buf_ = std::make_unique<internal::ObjDetectorBuffers>();
	impl_ = std::make_unique<internal::ObjDetectorImpl>();
	impl_->makeParams(cv::Scalar{scale[0], scale[1], scale[2]},
					  cv::Size{size[0], size[1]},
					  cv::Scalar{mean[0], mean[1], mean[2]}, swapRB,
					  enums::from<cv::dnn::ImagePaddingMode>(resizeMode_),
					  cv::Scalar{padValue[0], padValue[1], padValue[2]});
	return impl_->makeNet(std::filesystem::path{modelPath} / model,
						  enums::from<cv::dnn::Backend>(backend),
						  enums::from<cv::dnn::Target>(target));
}

}  // namespace beholder
