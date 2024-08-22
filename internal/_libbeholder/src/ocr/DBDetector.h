/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A DNN-based text detector wrapper using the DB model.
	For more info, see: https://github.com/MhLiao/DB

	WARNING: operates on images sized as multiples of 80px.

SourceFiles
	DBDetector.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_DB_DETECTOR_H
#define BEHOLDER_DB_DETECTOR_H

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
                      Class DBDetector Declaration
\*---------------------------------------------------------------------------*/

class DBDetector
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

		//- Binary map threshold
		float binaryThreshold {0.3};
		//- Text polygons threshold.
		//	Values are usually between 0.5 and 0.7.
		float polygonThreshold {0.5};
		//- Max number of output results
		int maxCandidates {50};
		//- Unclip ratio of the detected text region, which determines
		//	the output size
		double unclipRatio {2.0};

	// Constructors

		//- Default constructor
		DBDetector();

		//- Disable copy constructor
		DBDetector(const DBDetector&) = delete;

		//- Disable move constructor
		DBDetector(DBDetector&&) = delete;


	//- Destructor
	virtual ~DBDetector() = default;

	// Member functions

		//- Initialize a DBDetector
		virtual bool init() override;

	// Member operators

		//- Disable copy assignment
		DBDetector& operator=(const DBDetector&) = delete;

		//- Disable move assignment
		DBDetector& operator=(DBDetector&&) = delete;

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
