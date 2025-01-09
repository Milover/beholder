/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	An image brightness and contrast normalization operation.

SourceFiles
	FastNlMeansDenoise.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_FAST_NL_MEANS_DENOISE_OP_H
#define BEHOLDER_FAST_NL_MEANS_DENOISE_OP_H

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
                        Class FastNlMeansDenoise Declaration
\*---------------------------------------------------------------------------*/

class FastNlMeansDenoise
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

		//- New image dimensions
		float weight;

	//- Constructors

		//- Default constructor
		FastNlMeansDenoise() = default;

		//- Default constructor
		FastNlMeansDenoise(float w)
		:
			weight {w}
		{}

		//- Default copy constructor
		FastNlMeansDenoise(const FastNlMeansDenoise&) = default;

		//- Default move constructor
		FastNlMeansDenoise(FastNlMeansDenoise&&) = default;

	//- Destructor
	virtual ~FastNlMeansDenoise() = default;

	//- Member functions

	//- Member operators

		//- Default copy assignment
		FastNlMeansDenoise& operator=(const FastNlMeansDenoise&) = default;

		//- Default move assignment
		FastNlMeansDenoise& operator=(FastNlMeansDenoise&&) = default;

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
