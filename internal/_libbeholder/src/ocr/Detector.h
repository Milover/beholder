/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A DNN-based text detector wrapper.

	For more info on the DB model, see: https://github.com/MhLiao/DB
	For more info on the EAST model, see: https://github.com/argman/EAST

	WARNING: both DB and EAST models operate on images sized as
	multiples of 160px.

SourceFiles
	Detector.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_DETECTOR_H
#define BEHOLDER_DETECTOR_H

#ifndef CV__DNN_INLINE_NS
#define CV__DNN_INLINE_NS_BEGIN
#define CV__DNN_INLINE_NS_END
#endif

#include <string>
#include <vector>

#include "RawImage.h"
#include "Result.h"

// * * * * * * * * * * * * * Forward Declarations  * * * * * * * * * * * * * //

namespace cv
{
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

	using TDN = cv::dnn::TextDetectionModel;

private:

	// Private data

		//- A pointer to the tesseract api
		//	NOTE: a unique_ptr would be nicer, but cgo keeps complaining
		TDN* m_ {nullptr};

		//- OCR (detection) results
		std::vector<Result> res_;

		//- Multiplier for image pixel values.
		//	Set based on which model is in use.
		//	XXX: why tho?
		double scale_;

		//- Swap red and green channels
		bool swapRB_;

public:

	// Public data

		//- Directory path of the model (trained data) file
		std::string modelPath;
		//- Model (trained data) name
		std::string model;


	// Constructors

		//- Default constructor
		Detector();

		//- Disable copy constructor
		Detector(const Detector&) = delete;

		//- Disable move constructor
		Detector(Detector&&) = delete;


	//- Destructor
	~Detector();

	// Member functions

		//- Reset Detector and OCR results
		void clear();

		//- Run text detection and store the results
		bool detect(const RawImage& raw);

		//- Get a const reference to the OCR (detection) results
		const std::vector<Result>& getResults() const;

		//- Initialize Detector
		bool init();

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
