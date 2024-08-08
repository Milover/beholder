/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	An image brightness and contrast normalization operation.

SourceFiles
	MedianBlur.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_MEDIAN_BLUR_OP_H
#define BEHOLDER_MEDIAN_BLUR_OP_H

#include "ProcessingOp.h"

// * * * * * * * * * * * * * Forward Declarations  * * * * * * * * * * * * * //

namespace cv
{
	class Mat;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{

/*---------------------------------------------------------------------------*\
                        Class MedianBlur Declaration
\*---------------------------------------------------------------------------*/

class MedianBlur
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
			const Result&
		) const override;

public:

	//- Public data

		//- Kernel size
		int kernelSize {3};

	//- Constructors

		//- Default constructor
		MedianBlur() = default;

		//- Default constructor
		MedianBlur(int ksize)
		:
			ProcessingOp(),
			kernelSize {ksize}
		{}

	//- Member functions

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
