/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A DNN-based object detector wrapper.

SourceFiles
	ObjDetector.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_OBJ_DETECTOR_H
#define BEHOLDER_OBJ_DETECTOR_H

#ifndef CV__DNN_INLINE_NS
#define CV__DNN_INLINE_NS_BEGIN
#define CV__DNN_INLINE_NS_END
#endif

#include <array>
#include <memory>
#include <string>
#include <vector>

#include "RawImage.h"
#include "Result.h"

// * * * * * * * * * * * * * Forward Declarations  * * * * * * * * * * * * * //

namespace cv
{
	class Mat;

	namespace dnn
	{
		CV__DNN_INLINE_NS_BEGIN
			class Net;
			class Image2BlobParams;
		CV__DNN_INLINE_NS_END
	}
}

namespace beholder
{
	namespace internal
	{
		class ObjDetectorBuffers;
	}
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{

// can't do enum class because Go will complain.
enum NNBackend
{
	BackendDefault = 0,
	BackendHalide,
	BackendOpenVINO,
	BackendOpenCV,
	BackendVulkan,
	BackendCUDA,
	BackendWebNN,		// Microsoft WebNN
	BackendTIMVX,		// VeriSilicon TIM-VX
	BackendCANN,		// Huawei CANN backend
};

// can't do enum class because Go will complain
enum NNTarget
{
	TargetCPU = 0,
	TargetOpenCL,
	TargetOpenCLfp16,
	TargetMyriad,		// Xailient AI Myriad 2/X
	TargetVulkan,
	TargetFPGA,
	TargetCUDA,
	TargetCUDAfp16,
	TargetHDDL,			// OpenVINO HDDL
	TargetNPU,
	TargetCPUfp16,		// ARM only
};


/*---------------------------------------------------------------------------*\
                      Class ObjDetector Declaration
\*---------------------------------------------------------------------------*/

class ObjDetector
{
protected:

	// Protected data

		//- A pointer to an OpenCV DNN
		//	NOTE: a unique_ptr would be nicer, but cgo keeps complaining
		std::unique_ptr<cv::dnn::Net> net_ {nullptr};

		//- Processing parameters of image to blob.
		//	NOTE: a unique_ptr would be nicer, but cgo keeps complaining
		std::unique_ptr<cv::dnn::Image2BlobParams> params_ {nullptr};

		//- Buffers for temporary values used during detection
		//	WARNING: Go might complain because of the unique_ptr
		std::unique_ptr<internal::ObjDetectorBuffers> buf_ {nullptr};

		//- detection results
		std::vector<Result> res_;

	// Protected member functions

		//- Extract inference results
		//	TODO: we would like to time this externally, somehow
		//	TODO: should return an error of some kind
		virtual void extract() = 0;

public:

	// Public data

		//- Directory path of the model (weights) file.
		std::string modelPath;
		//- Model (weights) name.
		//
		//	NOTE: the model should be in ONNX format.
		std::string model;

		//- The network computation backend, see:
		//	https://docs.opencv.org/4.10.0/d6/d0f/group__dnn.html#ga186f7d9bfacac8b0ff2e26e2eab02625
		//
		//	NOTE: OpenCV needs to be compiled with additional backend support,
		//	eg. CUDA/CUDNN, OpenVINO...
		int backend {NNBackend::BackendDefault};
		//- The network target device for computations, see:
		//	https://docs.opencv.org/4.10.0/d6/d0f/group__dnn.html#ga709af7692ba29788182cf573531b0ff5
		int target {NNTarget::TargetCPU};

		//- A list of object clases that the loaded model supports.
		//	TODO: shouldn't be here
		std::vector<std::string> classes;

		//- Image size in px.
		//	The image will be resized and padded so that the largest image
		//	dimension is equal to size. The aspect ratio is preserved using
		//	letterboxing.
		int size {640};
		//- Multiplier for image pixel values.
		//	Set based on which model is in use.
		double scale {1.0/255.0};
		//- Confidence threshold used for text box filtering.
		float confidenceThreshold {0.5};
		//- Non-maximum suppression threshold.
		float nmsThreshold {0.4};
		//- Normalization constant.
		//	This value is subtracted from each pixel value of the current image.
        std::array<double, 3> mean {0.0, 0.0, 0.0};
		//- Swap red and green channels
        bool swapRB {true};
		//- The pixel value used to pad the image, if padding is active.
        std::array<double, 3> padValue {0.0, 0.0, 0.0};	// or 114.0, 114.0, 114.0


	// Constructors

		//- Default constructor
		ObjDetector();

		//- Disable copy constructor
		ObjDetector(const ObjDetector&) = default;

		//- Disable move constructor
		ObjDetector(ObjDetector&&) = default;


	//- Destructor
	virtual ~ObjDetector();

	// Member functions

		//- Clear detection results
		virtual void clear();

		//- Run text detection and store the results
		virtual bool detect(const RawImage& raw);

		//- Get a const reference to the detection results
		const std::vector<Result>& getResults() const;

		//- Initialize the object detector.
		virtual bool init();


	// Member operators

		//- Disable copy assignment
		ObjDetector& operator=(const ObjDetector&) = default;

		//- Disable move assignment
		ObjDetector& operator=(ObjDetector&&) = default;

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
