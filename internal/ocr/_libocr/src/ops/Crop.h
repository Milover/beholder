/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	An image cropping operation.

SourceFiles
	Crop.cpp

\*---------------------------------------------------------------------------*/

#ifndef OCR_CROP_OP_H
#define OCR_CROP_OP_H

#include "ProcessingOp.h"

// * * * * * * * * * * * * * Forward Declarations  * * * * * * * * * * * * * //

namespace cv
{
	class Mat;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace ocr
{

/*---------------------------------------------------------------------------*\
                        Class Crop Declaration
\*---------------------------------------------------------------------------*/

class Crop
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
			const OcrResults&
		) const override;

public:

	//- Public data

		//- Crop boundaries (inclusive)
		int left;
		int right;
		int top;
		int bottom;

	//- Constructors

		//- Default constructor
		Crop() = default;

		//- Default constructor
		Crop(int l, int r, int t, int b)
		:
			ProcessingOp(),
			left {l},
			right {r},
			top {t},
			bottom {b}
		{}

	//- Member functions

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace ocr

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
