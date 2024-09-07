/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	An image sharpennig operation using the 'unsharp mask' algorithm.

	For more info, see:
	https://docs.opencv.org/4.10.0/d1/d10/classcv_1_1MatExpr.html#details

SourceFiles
	UnsharpMask.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_UNSHARP_MASK_OP_H
#define BEHOLDER_UNSHARP_MASK_OP_H

#include <vector>

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
                        Class UnsharpMask Declaration
\*---------------------------------------------------------------------------*/

class UnsharpMask
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
			const std::vector<Result>&
		) const override;

public:

	//- Public data

		//- Scale factor
		double sigma {255.0};
		double threshold {255.0};
		double amount {255.0};

	//- Constructors

		//- Default constructor
		UnsharpMask() = default;

		//- Default constructor
		UnsharpMask(double s = 1.0, double t = 5.0, double a = 1.0)
		:
			sigma {s},
			threshold {t},
			amount {a}
		{}

		//- Default copy constructor
		UnsharpMask(const UnsharpMask&) = default;

		//- Default move constructor
		UnsharpMask(UnsharpMask&&) = default;

	//- Destructor
	virtual ~UnsharpMask() = default;

	//- Member functions

	//- Member operators

		//- Default copy assignment
		UnsharpMask& operator=(const UnsharpMask&) = default;

		//- Default move assignment
		UnsharpMask& operator=(UnsharpMask&&) = default;

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
