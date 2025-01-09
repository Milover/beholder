/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	An image rotation operation.

SourceFiles
	Rotate.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_ROTATE_OP_H
#define BEHOLDER_ROTATE_OP_H

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
                        Class Rotate Declaration
\*---------------------------------------------------------------------------*/

class Rotate
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

		//- Angle of rotation
		float angle;

	//- Constructors

		//- Default constructor
		Rotate() = default;

		//- Default constructor
		Rotate(float ang)
		:
			ProcessingOp(),
			angle {ang}
		{}

		//- Default copy constructor
		Rotate(const Rotate&) = default;

		//- Default move constructor
		Rotate(Rotate&&) = default;

	//- Destructor
	virtual ~Rotate() = default;

	//- Member functions

	//- Member operators

		//- Default copy assignment
		Rotate& operator=(const Rotate&) = default;

		//- Default move assignment
		Rotate& operator=(Rotate&&) = default;

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
