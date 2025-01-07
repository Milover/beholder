/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	An image cropping operation.

SourceFiles
	Crop.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_CROP_OP_H
#define BEHOLDER_CROP_OP_H

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
			const std::vector<Result>&
		) const override;

public:

	//- Public data

		//- Crop boundaries (inclusive)
		int left;
		int top;
		int width;
		int height;

	//- Constructors

		//- Default constructor
		Crop() = default;

		//- Default constructor
		Crop(int l, int t, int w, int h)
		:
			ProcessingOp(),
			left {l},
			top {t},
			width {w},
			height {h}
		{}

		//- Default copy constructor
		Crop(const Crop&) = default;

		//- Default move constructor
		Crop(Crop&&) = default;

	//- Destructor
	virtual ~Crop() = default;

	//- Member functions

	//- Member operators

		//- Default copy assignment
		Crop& operator=(const Crop&) = default;

		//- Default move assignment
		Crop& operator=(Crop&&) = default;

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
