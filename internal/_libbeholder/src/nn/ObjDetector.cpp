/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <filesystem>
#include <string>
#include <vector>

#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/dnn/dnn.hpp>

#include "ObjDetector.h"
#include "Processor.h"
#include "RawImage.h"
#include "Rectangle.h"
#include "Result.h"

#include "internal/ObjDetectorBuffers.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{

// * * * * * * * * * * * * * * * Static Checks * * * * * * * * * * * * * * * //

// DNN backend checks
static_assert(static_cast<int>(NNBackend::BackendDefault) == static_cast<int>(cv::dnn::DNN_BACKEND_DEFAULT));
static_assert(static_cast<int>(NNBackend::BackendHalide) == static_cast<int>(cv::dnn::DNN_BACKEND_HALIDE));
static_assert(static_cast<int>(NNBackend::BackendOpenVINO) == static_cast<int>(cv::dnn::DNN_BACKEND_INFERENCE_ENGINE));
static_assert(static_cast<int>(NNBackend::BackendOpenCV) == static_cast<int>(cv::dnn::DNN_BACKEND_OPENCV));
static_assert(static_cast<int>(NNBackend::BackendVulkan) == static_cast<int>(cv::dnn::DNN_BACKEND_VKCOM));
static_assert(static_cast<int>(NNBackend::BackendCUDA) == static_cast<int>(cv::dnn::DNN_BACKEND_CUDA));
static_assert(static_cast<int>(NNBackend::BackendWebNN) == static_cast<int>(cv::dnn::DNN_BACKEND_WEBNN));
static_assert(static_cast<int>(NNBackend::BackendTIMVX) == static_cast<int>(cv::dnn::DNN_BACKEND_TIMVX));
static_assert(static_cast<int>(NNBackend::BackendCANN) == static_cast<int>(cv::dnn::DNN_BACKEND_CANN));

// DNN target checks
static_assert(static_cast<int>(NNTarget::TargetCPU) == 	static_cast<int>(cv::dnn::DNN_TARGET_CPU));
static_assert(static_cast<int>(NNTarget::TargetOpenCL) == static_cast<int>(cv::dnn::DNN_TARGET_OPENCL));
static_assert(static_cast<int>(NNTarget::TargetOpenCLfp16) == static_cast<int>(cv::dnn::DNN_TARGET_OPENCL_FP16));
static_assert(static_cast<int>(NNTarget::TargetMyriad) == static_cast<int>(cv::dnn::DNN_TARGET_MYRIAD));
static_assert(static_cast<int>(NNTarget::TargetVulkan) == static_cast<int>(cv::dnn::DNN_TARGET_VULKAN));
static_assert(static_cast<int>(NNTarget::TargetFPGA) == static_cast<int>(cv::dnn::DNN_TARGET_FPGA));
static_assert(static_cast<int>(NNTarget::TargetCUDA) == static_cast<int>(cv::dnn::DNN_TARGET_CUDA));
static_assert(static_cast<int>(NNTarget::TargetCUDAfp16) == static_cast<int>(cv::dnn::DNN_TARGET_CUDA_FP16));
static_assert(static_cast<int>(NNTarget::TargetHDDL) == static_cast<int>(cv::dnn::DNN_TARGET_HDDL));
static_assert(static_cast<int>(NNTarget::TargetNPU) == static_cast<int>(cv::dnn::DNN_TARGET_NPU));
static_assert(static_cast<int>(NNTarget::TargetCPUfp16) == static_cast<int>(cv::dnn::DNN_TARGET_CPU_FP16));

// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

// * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * * //

ObjDetector::ObjDetector()
{}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

ObjDetector::~ObjDetector()
{}

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

void ObjDetector::clear()
{
	if (buf_)
	{
		buf_->clear();
	}
	res_.clear();
}

bool ObjDetector::detect(const capi::RawImage& raw)
{
	clear();

	if (net_->empty())
	{
		return false;
	}
	auto img {rawToMatPtr(raw)};
	if (!img)
	{
		return false;
	}

	// preprocess
	cv::dnn::blobFromImageWithParams(*img, buf_->blob, *params_);

	// forward
	net_->setInput(buf_->blob);
	// TODO: not sure how clear() and buf_->outs impact re-allocation,
	// should check at some point.
	// Ideally we would want to keep the memory allocated by
	// each of the buf_->outs, instead of throwing it away each time.
	net_->forward(buf_->outs, net_->getUnconnectedOutLayersNames());

	// extract and prep raw results for storage
	extract();
	// TODO: will probably have to re-implement at some point because we
	// would like this to work for RotatedRects as well.
	// See TextDetectionModel_EAST_impl::detectTextRectangles for an example
	// implamentation.
	params_->blobRectsToImageRects(buf_->tBoxes, buf_->tBoxes, img->size());

	// filter and store results
	res_.reserve(buf_->tNMSIDs.size());
	std::string text {""};
	for (auto i {0ul}; i < buf_->tNMSIDs.size(); ++i)
	{
		auto id {buf_->tNMSIDs[i]};	// use NMS filtered IDs to select results
		const cv::Rect& r {buf_->tBoxes[id]};
		if (!buf_->tClassIDs.empty())
		{
			auto classID {buf_->tClassIDs[id]};
			text = classes.size() > static_cast<size_t>(classID)
				 ? classes[classID] : std::to_string(classID);
		}
		res_.emplace_back
		(
			Result
			{
				text,
				capi::Rectangle {r.x, r.y, r.x + r.width, r.y + r.height},
				static_cast<double>(buf_->tConfidences[id])
			}
		);
	}
	return !res_.empty();
}

const std::vector<Result>& ObjDetector::getResults() const
{
	return res_;
}

bool ObjDetector::init()
{
	// XXX: no checks, we assume that it's been checked and is correct; yolo
	// must be an '.onnx' file though
	std::filesystem::path modelFile
	{
		std::filesystem::path{modelPath} / model
	};
	// TODO: we could supply a buffer and read from memory
	//net_.reset(new cv::dnn::Net {cv::dnn::readNetFromONNX(modelFile.string())});
	net_.reset(new cv::dnn::Net {cv::dnn::readNet(modelFile.string())});
    net_->setPreferableBackend(backend);
    net_->setPreferableTarget(target);
	params_.reset
	(
		new cv::dnn::Image2BlobParams
		{
			cv::Scalar::all(scale),
			cv::Size {size, size},
			cv::Scalar {mean[0], mean[1], mean[2]},
			swapRB,
			CV_32F,
			cv::dnn::DNN_LAYOUT_NCHW,
			cv::dnn::DNN_PMODE_LETTERBOX,
			cv::Scalar {padValue[0], padValue[1], padValue[2]}
		}
	);
	buf_.reset(new internal::ObjDetectorBuffers {});

	return !net_->empty();
}

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// ************************************************************************* //
