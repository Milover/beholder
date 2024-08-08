/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A background removal operation by means of Gaussian blur division.

SourceFiles
	DivGaussianBlur.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_DIV_GAUSSIAN_BLUR_OP_H
#define BEHOLDER_DIV_GAUSSIAN_BLUR_OP_H

#include "GaussianBlur.h"

// * * * * * * * * * * * * * Forward Declarations  * * * * * * * * * * * * * //

namespace cv
{
	class Mat;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{

/*---------------------------------------------------------------------------*\
                        Class DivGaussianBlur Declaration
\*---------------------------------------------------------------------------*/

class DivGaussianBlur
:
	public GaussianBlur
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
			const Result&
		) const override;

public:

	//- Public data

		//- Scale factor
		float scaleFactor {255.0};

	//- Constructors

		//- Default constructor
		DivGaussianBlur() = default;

		//- Default constructor
		DivGaussianBlur
		(
			float scale = 255.0,
			float sX = 33.0,
			float sY = 33.0,
			int kW = 0,
			int kH = 0
		)
		:
			GaussianBlur(kW, kH, sX, sY),
			scaleFactor {scale}
		{}

	//- Member functions

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
