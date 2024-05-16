/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	An image brightness and contrast normalization operation.

SourceFiles
	Resize.cpp

\*---------------------------------------------------------------------------*/

#ifndef OCR_RESIZE_OP_H
#define OCR_RESIZE_OP_H

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
                        Class Resize Declaration
\*---------------------------------------------------------------------------*/

class Resize
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

		//- New image dimensions
		int width;
		int height;

	//- Constructors

		//- Default constructor
		Resize() = default;

		//- Default constructor
		Resize(int w, int h)
		:
			ProcessingOp(),
			width {w},
			height {h}
		{}

	//- Member functions

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace ocr

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
