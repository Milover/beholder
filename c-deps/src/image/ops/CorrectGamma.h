/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A gamma correction operation.

SourceFiles
	CorrectGamma.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_CORRECT_GAMMA_OP_H
#define BEHOLDER_CORRECT_GAMMA_OP_H

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
                         Class CorrectGamma Declaration
\*---------------------------------------------------------------------------*/

class CorrectGamma
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

		//- Gamma correction value
		double gamma {1.0};

	//- Constructors

		//- Default constructor
		CorrectGamma() = default;

		//- Default constructor
		CorrectGamma(double g)
		:
			gamma {g}
		{}

		//- Default copy constructor
		CorrectGamma(const CorrectGamma&) = default;

		//- Default move constructor
		CorrectGamma(CorrectGamma&&) = default;

	//- Destructor
	virtual ~CorrectGamma() = default;

	//- Member functions

	//- Member operators

		//- Default copy assignment
		CorrectGamma& operator=(const CorrectGamma&) = default;

		//- Default move assignment
		CorrectGamma& operator=(CorrectGamma&&) = default;

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
