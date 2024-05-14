/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	An automatic image cropping operation.

SourceFiles
	AutoCrop.cpp

\*---------------------------------------------------------------------------*/

#ifndef OCR_AUTO_CROP_OP_H
#define OCR_AUTO_CROP_OP_H

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
                        Class AutoCrop Declaration
\*---------------------------------------------------------------------------*/

class AutoCrop
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

		//- Kernel size
		int kernelSize {50};
		//- Dimensions used for text box detection.
		//	Potential text boxes with smaller dimensions are discarded.
		float textHeight {50};
		float textWidth {50};
		//- Padding added to the cropped image
		float padding {10.0};
		//- Gradient kernel size
		const int gradientKernelSize {3};

	//- Constructors

		//- Default constructor
		AutoCrop() = default;

		//- Default constructor
		AutoCrop(int kS, float tH, float tW, float pad)
		:
			ProcessingOp(),
			kernelSize {kS},
			textHeight {tH},
			textWidth {tW},
			padding {pad}
		{}

	//- Member functions

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace ocr

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
