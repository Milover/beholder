/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A DNN-based text detector wrapper.

	WARNING: most models operate on images sized as multiples of 80px.

SourceFiles
	Detector.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_DETECTOR_H
#define BEHOLDER_DETECTOR_H

#ifndef CV__DNN_INLINE_NS
#define CV__DNN_INLINE_NS_BEGIN
#define CV__DNN_INLINE_NS_END
#endif

#include <array>
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
			class TextDetectionModel;
		CV__DNN_INLINE_NS_END
	}
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{

/*---------------------------------------------------------------------------*\
                      Class Detector Declaration
\*---------------------------------------------------------------------------*/

class Detector
{
public:

	using TDM = cv::dnn::TextDetectionModel;

private:

	// Private data

		//- A pointer to the OpenCV text detector
		//	NOTE: a unique_ptr would be nicer, but cgo keeps complaining
		TDM* p_ {nullptr};

		//- OCR (detection) results
		std::vector<Result> res_;

protected:

	// Protected data

		//- Multiplier for image pixel values.
		//	Set based on which model is in use.
		//	XXX: why tho?
		double scale_ {1.0};

		//- Swap red and green channels
		bool swapRB_ {false};


	// Protected member functions

		//- Get the mean value which is subtracted from the image.
		virtual std::array<double, 3> getMean(const cv::Mat& img) const;

		//- Is the underlying text detector pointer valid?
		bool hasPtr() const;

		//- Delete and reset the underlying text detector pointer.
		void resetPtr(TDM* p = nullptr);

public:

	// Public data

		//- Directory path of the model (trained data) file
		std::string modelPath;
		//- Model (trained data) name
		std::string model;
		//- Use a hard-coded mean value. For more info, see:
		//	https://docs.opencv.org/4.10.0/d4/d43/tutorial_dnn_text_spotting.html
		bool useHardCodedMean {false};

		//- dimLCD is the least common denominator of the image dimensions, i.e.
		//	the image dimensions must be a multiple of dimLCD for the detector
		//	to work properly.
		inline static constexpr int dimLCD {80};


	// Constructors

		//- Default constructor
		Detector() = default;

		//- Disable copy constructor
		Detector(const Detector&) = delete;

		//- Disable move constructor
		Detector(Detector&&) = delete;


	//- Destructor
	virtual ~Detector();

	// Member functions

		//- Reset Detector and OCR results
		void clear();

		//- Run text detection and store the results
		bool detect(const RawImage& raw);

		//- Get a const reference to the OCR (detection) results
		const std::vector<Result>& getResults() const;

		//- Initialize a Detector
		virtual bool init() = 0;

	// Member operators

		//- Disable copy assignment
		Detector& operator=(const Detector&) = delete;

		//- Disable move assignment
		Detector& operator=(Detector&&) = delete;

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
