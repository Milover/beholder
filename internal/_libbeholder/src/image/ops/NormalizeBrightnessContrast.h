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
			const std::vector<Result>&
		) const override;

public:

	//- Public data

		//- Low-value side clip percentage
		float clipLowPct {0.25};
		//- High-value side percentage
		float clipHighPct {0.25};

	//- Constructors

		//- Default constructor
		NormalizeBrightnessContrast() = default;

		//- Default constructor
		//	Clip 'cPct' of image values in total, symmetrically from both sides
		NormalizeBrightnessContrast(float cPct)
		:
			ProcessingOp(),
			clipLowPct {cPct/2.0f},
			clipHighPct {cPct/2.0f}
		{}

		//- Default constructor
		NormalizeBrightnessContrast(float cLowPct, float cHighPct)
		:
			ProcessingOp(),
			clipLowPct {cLowPct},
			clipHighPct {cHighPct}
		{}

		//- Default copy constructor
		NormalizeBrightnessContrast(const NormalizeBrightnessContrast&) = default;

		//- Default move constructor
		NormalizeBrightnessContrast(NormalizeBrightnessContrast&&) = default;

	//- Destructor
	virtual ~NormalizeBrightnessContrast() = default;

	//- Member functions

	//- Member operators

		//- Default copy assignment
		NormalizeBrightnessContrast& operator=(const NormalizeBrightnessContrast&) = default;

		//- Default move assignment
		NormalizeBrightnessContrast& operator=(NormalizeBrightnessContrast&&) = default;

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

//- Normalize brightness and contrast
//	Taken from: https://stackoverflow.com/a/56909036/17881968
//	TODO: this needs to be refactored
bool normalizeBrightnessContrast
(
	const cv::Mat& in,
	cv::Mat& out,
	float clipLowPct = 0.25,
	float clipHighPct = 0.25
);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
