/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	An image operation which, optionally, rotates the image by 90° clockwise,
	so that image width is greater than image height.
	If image width is already greater than image height, the operation does
	nothing.

SourceFiles
	Landscape.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_LANDSCAPE_OP_H
#define BEHOLDER_LANDSCAPE_OP_H

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
                          Class Landscape Declaration
\*---------------------------------------------------------------------------*/

class Landscape
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

	//- Constructors

		//- Default constructor
		Landscape() = default;

		//- Default copy constructor
		Landscape(const Landscape&) = default;

		//- Default move constructor
		Landscape(Landscape&&) = default;

	//- Destructor
	virtual ~Landscape() = default;

	//- Member functions

	//- Member operators

		//- Default copy assignment
		Landscape& operator=(const Landscape&) = default;

		//- Default move assignment
		Landscape& operator=(Landscape&&) = default;

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
