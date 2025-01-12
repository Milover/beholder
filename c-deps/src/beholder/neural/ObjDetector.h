// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

// A DNN-based object detector wrapper.

#ifndef BEHOLDER_NEURAL_OBJ_DETECTOR_H
#define BEHOLDER_NEURAL_OBJ_DETECTOR_H

#include <array>
#include <memory>
#include <string>
#include <vector>

#include "capi/Image.h"
#include "capi/Result.h"

namespace cv {
class Mat;

namespace dnn {
class Net;
class Image2BlobParams;
}  // namespace dnn
}  // namespace cv

namespace beholder {
namespace internal {
class ObjDetectorBuffers;
}
}  // namespace beholder

namespace beholder {

// WARNING: these are OpenCV supported backends, so they will get changed in the
// future.
// NOTE: can't do enum class because Go will complain.
enum class NNBackend {
	BackendDefault = 0,
	BackendHalide,
	BackendOpenVINO,
	BackendOpenCV,
	BackendVulkan,
	BackendCUDA,
	BackendWebNN,  // Microsoft WebNN
	BackendTIMVX,  // VeriSilicon TIM-VX
	BackendCANN	   // Huawei CANN backend
};

// WARNING: these are OpenCV supported targets, so they will get changed in the
// future.
// NOTE: can't do enum class because Go will complain
enum class NNTarget {
	TargetCPU = 0,
	TargetOpenCL,
	TargetOpenCLfp16,
	TargetMyriad,  // Xailient AI Myriad 2/X
	TargetVulkan,
	TargetFPGA,
	TargetCUDA,
	TargetCUDAfp16,
	TargetHDDL,	 // OpenVINO HDDL
	TargetNPU,
	TargetCPUfp16  // ARM only
};

class ObjDetector {
public:
	template<typename T = int>
	using Vec2 = std::array<T, 2>;
	template<typename T = double>
	using Vec3 = std::array<T, 3>;

	// Image padding/resizing mode during image-to-blob conversion.
	//
	// This should usually be set by the model, not at runtime, i.e.
	// Go shouldn't need to know these values, so we can keep them here.
	//
	// NOTE: the details of each of these depent on OpenCV's implementation
	// currently, so even though the methods are described below, the actual
	// implementation might differ.
	enum class ResizeMode {
		// Resize directly to 'size'.
		ResizeRaw = 0,
		// Resize based on the largest scale factor, determined by comparing
		// 'size' to the image size, and then crop to 'size' from the center.
		ResizeCrop,
		// Resize based on the smallest scale factor, determined by comparing
		// 'size' to the image size, and then letterbox to 'size'
		ResizeLetterbox
	};

protected:
	// A pointer to an OpenCV DNN
	// NOTE: a unique_ptr would be nicer, but cgo keeps complaining
	std::unique_ptr<cv::dnn::Net> net_{nullptr};

	// Processing parameters of image to blob.
	// NOTE: a unique_ptr would be nicer, but cgo keeps complaining
	std::unique_ptr<cv::dnn::Image2BlobParams> params_{nullptr};

	// Buffers for temporary values used during detection
	// WARNING: Go might complain because of the unique_ptr
	std::unique_ptr<internal::ObjDetectorBuffers> buf_{nullptr};

	// detection results
	std::vector<Result> res_;

	// Image padding/resize mode when converting to blob.
	// Should usually be set by the model, not at runtime.
	// TODO: should letterboxing be the default?
	ResizeMode resizeMode_{ResizeMode::ResizeLetterbox};

	// Extract and store inference results
	// TODO: we would like to time this externally, somehow
	// TODO: should return an error of some kind
	virtual void extract() = 0;

	// Store extracted results from the buffer after mapping bounding
	// boxes from the blob back to the image.
	virtual void store() = 0;

public:
	// Directory path of the model (weights) file.
	std::string modelPath;
	// Model (weights) name.
	// NOTE: the model should be in ONNX format.
	std::string model;

	// The network computation backend.
	// For more info, see:
	// https://docs.opencv.org/4.10.0/d6/d0f/group__dnn.html#ga186f7d9bfacac8b0ff2e26e2eab02625
	//
	// NOTE: OpenCV needs to be compiled with additional backend support,
	// eg. CUDA/CUDNN, OpenVINO...
	NNBackend backend{NNBackend::BackendDefault};
	// The network target device for computations.
	// For more info, see:
	// https://docs.opencv.org/4.10.0/d6/d0f/group__dnn.html#ga709af7692ba29788182cf573531b0ff5
	NNTarget target{NNTarget::TargetCPU};

	// A list of object clases that the loaded model supports.
	// TODO: shouldn't be here
	std::vector<std::string> classes;

	// NOLINTBEGIN(*-magic-numbers)

	// Image size in px.
	// The image will be resized and padded so that the largest image
	// dimension is equal to size. The aspect ratio is preserved using
	// letterboxing.
	Vec2<> size{640, 640};
	// Multiplier for image pixel values.
	// Set based on which model is in use.
	Vec3<> scale{1.0, 1.0, 1.0};
	// Confidence threshold used for text box filtering.
	float confidenceThreshold{0.5};
	// Non-maximum suppression threshold.
	float nmsThreshold{0.4};
	// Normalization constant.
	// This value is subtracted from each pixel value of the current image.
	Vec3<> mean{0.0, 0.0, 0.0};
	// Swap red and green channels
	bool swapRB{true};
	// The pixel value used to pad the image, if padding is active.
	Vec3<> padValue{0.0, 0.0, 0.0};

	// NOLINTEND(*-magic-numbers)

	// Default constructor
	ObjDetector() = default;

	ObjDetector(const ObjDetector&) = delete;
	ObjDetector(ObjDetector&&) = default;

	virtual ~ObjDetector() = default;

	ObjDetector& operator=(const ObjDetector&) = delete;
	ObjDetector& operator=(ObjDetector&&) = default;

	// Clear detection results.
	virtual void clear();

	// Run inferencing and store the results.
	// NOTE: the results are cleared as soon as detect is called.
	virtual bool detect(const Image& raw);

	// Get a const reference to the detection results.
	[[nodiscard]] const std::vector<Result>& getResults() const;

	// Initialize the object detector.
	virtual bool init();
};

}  // namespace beholder

#endif	// BEHOLDER_NEURAL_OBJ_DETECTOR_H
