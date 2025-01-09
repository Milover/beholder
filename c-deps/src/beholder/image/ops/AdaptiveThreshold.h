/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	An adaptive thresholding operation.

SourceFiles
	AdaptiveThreshold.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_ADAPTIVE_THRESHOLD_OP_H
#define BEHOLDER_ADAPTIVE_THRESHOLD_OP_H

#include <vector>

#include "image/ProcessingOp.h"

// * * * * * * * * * * * * * Forward Declarations  * * * * * * * * * * * * * //

namespace cv
{
	class Mat;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{

/*---------------------------------------------------------------------------*\
                        Class AdaptiveThreshold Declaration
\*---------------------------------------------------------------------------*/

class AdaptiveThreshold
:
	public ProcessingOp
{
protected:

	// Protected member functions

		//- Execute the processing operation
		bool execute(const cv::Mat& in, cv::Mat& out) const override;

		//- Execute the processing operation
		bool execute
		(
			const cv::Mat& in,
			cv::Mat& out,
			const std::vector<Result>&
		) const override;

public:

	//- Public data

		//- Max value
		double maxValue {255.0};
		//- Kernel size
		int size {11};	// XXX: magic
		//- A constant subtracted from the mean or weighted mean
		double c {2};	// XXX: magic
		//- AdaptiveThreshold type
		//	0 = cv::ADAPTIVE_THRESH_MEAN_C; 1 = cv::ADAPTIVE_THRESH_GAUSSIAN_C
		int type {1};

	//- Constructors

		//- Default constructor
		AdaptiveThreshold() = default;

		//- Default constructor
		AdaptiveThreshold(double max, int sz, double cnst, int typ)
		:
			maxValue {max},
			size {sz},
			c {cnst},
			type {typ}
		{}

		//- Default copy constructor
		AdaptiveThreshold(const AdaptiveThreshold&) = default;

		//- Default move constructor
		AdaptiveThreshold(AdaptiveThreshold&&) = default;

	//- Destructor
	virtual ~AdaptiveThreshold() = default;

	//- Member functions

	//- Member operators

		//- Default copy assignment
		AdaptiveThreshold& operator=(const AdaptiveThreshold&) = default;

		//- Default move assignment
		AdaptiveThreshold& operator=(AdaptiveThreshold&&) = default;

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
