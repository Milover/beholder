/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	An image brightness and contrast normalization operation.

SourceFiles
	ResizeToHeight.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_RESIZE_TO_HEIGHT_OP_H
#define BEHOLDER_RESIZE_TO_HEIGHT_OP_H

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
                        Class ResizeToHeight Declaration
\*---------------------------------------------------------------------------*/

class ResizeToHeight
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

		//- New image height
		int height;

	//- Constructors

		//- Default constructor
		ResizeToHeight() = default;

		//- Default constructor
		ResizeToHeight(int h)
		:
			height {h}
		{}

		//- Default copy constructor
		ResizeToHeight(const ResizeToHeight&) = default;

		//- Default move constructor
		ResizeToHeight(ResizeToHeight&&) = default;

	//- Destructor
	virtual ~ResizeToHeight() = default;

	//- Member functions

	//- Member operators

		//- Default copy assignment
		ResizeToHeight& operator=(const ResizeToHeight&) = default;

		//- Default move assignment
		ResizeToHeight& operator=(ResizeToHeight&&) = default;

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
