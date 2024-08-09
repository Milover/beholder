/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	An image brightness and contrast normalization operation.

SourceFiles
	GaussianBlur.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_GAUSSIAN_BLUR_OP_H
#define BEHOLDER_GAUSSIAN_BLUR_OP_H

#include <vector>

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
                        Class GaussianBlur Declaration
\*---------------------------------------------------------------------------*/

class GaussianBlur
:
	public ProcessingOp
{
protected:

	// Protected member functions

		//- Execute the processing operation
		virtual bool execute(const cv::Mat& in, cv::Mat& out) const override;

		//- Execute the processing operation
		virtual bool execute
		(
			const cv::Mat& in,
			cv::Mat& out,
			const std::vector<Result>&
		) const override;

public:

	//- Public data

		//- Kernel size
		int kernelWidth {3};
		int kernelHeight {3};
		//- Kernel standard deviation
		float sigmaX {0};
		float sigmaY {0};

	//- Constructors

		//- Default constructor
		GaussianBlur() = default;

		//- Default constructor
		GaussianBlur(int kW, int kH, float sX, float sY)
		:
			ProcessingOp(),
			kernelWidth {kW},
			kernelHeight {kH},
			sigmaX {sX},
			sigmaY {sY}
		{}

		//- Default copy constructor
		GaussianBlur(const GaussianBlur&) = default;

		//- Default move constructor
		GaussianBlur(GaussianBlur&&) = default;

	//- Destructor
	virtual ~GaussianBlur() = default;

	//- Member functions

	//- Member operators

		//- Default copy assignment
		GaussianBlur& operator=(const GaussianBlur&) = default;

		//- Default move assignment
		GaussianBlur& operator=(GaussianBlur&&) = default;

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
