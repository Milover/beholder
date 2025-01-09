/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	An image rescaling operation.

SourceFiles
	Rescale.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_RESCALE_OP_H
#define BEHOLDER_RESCALE_OP_H

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
                        Class Rescale Declaration
\*---------------------------------------------------------------------------*/

class Rescale
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

		//- The scale factor
		double scale;

	//- Constructors

		//- Default constructor
		Rescale() = default;

		//- Default constructor
		Rescale(double s)
		:
			scale {s}
		{}

		//- Default copy constructor
		Rescale(const Rescale&) = default;

		//- Default move constructor
		Rescale(Rescale&&) = default;

	//- Destructor
	virtual ~Rescale() = default;

	//- Member functions

	//- Member operators

		//- Default copy assignment
		Rescale& operator=(const Rescale&) = default;

		//- Default move assignment
		Rescale& operator=(Rescale&&) = default;

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
