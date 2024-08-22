/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A DNN-based text detector wrapper using the EAST model.
	For more info, see: https://github.com/argman/EAST

	WARNING: operates on images sized as multiples of 80px.

SourceFiles
	EASTDetector.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_EAST_DETECTOR_H
#define BEHOLDER_EAST_DETECTOR_H

#include <array>

#include "Detector.h"

// * * * * * * * * * * * * * Forward Declarations  * * * * * * * * * * * * * //

namespace cv
{
	class Mat;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{

/*---------------------------------------------------------------------------*\
                      Class EASTDetector Declaration
\*---------------------------------------------------------------------------*/

class EASTDetector
:
	public Detector
{
public:

	using Base = Detector;

protected:

	// Protected member functions

		//- Get the mean value which is subtracted from the image.
		virtual std::array<double, 3> getMean(const cv::Mat& img) const override;

public:

	// Public data

		//- Confidence threshold used for text box filtering.
		float confidenceThreshold {0.5};
		//- Non-maximum suppression threshold.
		float nmsThreshold {0.0};

	// Constructors

		//- Default constructor
		EASTDetector();

		//- Disable copy constructor
		EASTDetector(const EASTDetector&) = delete;

		//- Disable move constructor
		EASTDetector(EASTDetector&&) = delete;


	//- Destructor
	virtual ~EASTDetector() = default;

	// Member functions

		//- Initialize a EASTDetector
		virtual bool init() override;

	// Member operators

		//- Disable copy assignment
		EASTDetector& operator=(const EASTDetector&) = delete;

		//- Disable move assignment
		EASTDetector& operator=(EASTDetector&&) = delete;

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
