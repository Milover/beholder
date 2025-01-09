/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	An image brightness and contrast normalization operation.

SourceFiles
	Invert.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_INVERT_OP_H
#define BEHOLDER_INVERT_OP_H

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
                        Class Invert Declaration
\*---------------------------------------------------------------------------*/

class Invert
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

		//- Default constructor
		Invert() = default;

		//- Default copy constructor
		Invert(const Invert&) = default;

		//- Default move constructor
		Invert(Invert&&) = default;

	//- Destructor
	virtual ~Invert() = default;

	//- Member functions

	//- Member operators

		//- Default copy assignment
		Invert& operator=(const Invert&) = default;

		//- Default move assignment
		Invert& operator=(Invert&&) = default;

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
