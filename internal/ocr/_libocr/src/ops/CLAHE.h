/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A contrast limited adaptive histogram equalization operation

SourceFiles
	CLAHE.cpp

\*---------------------------------------------------------------------------*/

#ifndef OCR_CLAHE_OP_H
#define OCR_CLAHE_OP_H

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
                            Class CLAHE Declaration
\*---------------------------------------------------------------------------*/

class CLAHE
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

		//- Clip percentage
		float clipLimit {40.0};
		//- No. tile rows
		int tileRows {8};
		//- No. tile columns
		int tileColumns {8};

	//- Constructors

		//- Default constructor
		CLAHE() = default;

		//- Default constructor
		CLAHE(float cLim, int tR, int tC)
		:
			ProcessingOp(),
			clipLimit {cLim},
			tileRows {tR},
			tileColumns {tC}
		{}

	//- Member functions

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace ocr

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
