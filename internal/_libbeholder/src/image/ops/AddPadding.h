/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	An operation which adds uniform padding around the border of an image

SourceFiles
	AddPadding.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_ADD_PADDING_OP_H
#define BEHOLDER_ADD_PADDING_OP_H

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
                        Class AddPadding Declaration
\*---------------------------------------------------------------------------*/

class AddPadding
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

		//- Kernel size
		int padding {10};

	//- Constructors

		//- Default constructor
		AddPadding() = default;

		//- Default constructor
		AddPadding(int pad)
		:
			ProcessingOp(),
			padding {pad}
		{}

	//- Member functions

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
