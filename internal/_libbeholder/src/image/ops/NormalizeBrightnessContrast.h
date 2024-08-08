/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	An image brightness and contrast normalization operation.

SourceFiles
	NormalizeBrightnessContrast.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_NORMALIZE_BRIGHTNESS_CONTRAST_OP_H
#define BEHOLDER_NORMALIZE_BRIGHTNESS_CONTRAST_OP_H

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
               Class NormalizeBrightnessContrast Declaration
\*---------------------------------------------------------------------------*/

class NormalizeBrightnessContrast
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

		//- Clip percentage
		float clipPct {0.5};

	//- Constructors

		//- Default constructor
		NormalizeBrightnessContrast() = default;

		//- Default constructor
		NormalizeBrightnessContrast(float cPct)
		:
			ProcessingOp(),
			clipPct {cPct}
		{}

	//- Member functions

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

//- Normalize brightness and contrast
//	Taken from: https://stackoverflow.com/a/56909036/17881968
//	TODO: this needs to be refactored
bool normalizeBrightnessContrast
(
	const cv::Mat& in,
	cv::Mat& out,
	float clipPct = 0.5
);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
