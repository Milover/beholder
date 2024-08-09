/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	An image brightness and contrast normalization operation.

SourceFiles
	Resize.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_RESIZE_OP_H
#define BEHOLDER_RESIZE_OP_H

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
			const std::vector<Result>&
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

		//- Default copy constructor
		Resize(const Resize&) = default;

		//- Default move constructor
		Resize(Resize&&) = default;

	//- Destructor
	virtual ~Resize() = default;

	//- Member functions

	//- Member operators

		//- Default copy assignment
		Resize& operator=(const Resize&) = default;

		//- Default move assignment
		Resize& operator=(Resize&&) = default;

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
