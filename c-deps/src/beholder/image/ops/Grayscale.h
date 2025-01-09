/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	An image-to-grayscale-image conversion operation.

SourceFiles
	Grayscale.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_GRAYSCALE_OP_H
#define BEHOLDER_GRAYSCALE_OP_H

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
                        Class Grayscale Declaration
\*---------------------------------------------------------------------------*/

class Grayscale
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
		Grayscale() = default;

		//- Default copy constructor
		Grayscale(const Grayscale&) = default;

		//- Default move constructor
		Grayscale(Grayscale&&) = default;

	//- Destructor
	virtual ~Grayscale() = default;

	//- Member functions

	//- Member operators

		//- Default copy assignment
		Grayscale& operator=(const Grayscale&) = default;

		//- Default move assignment
		Grayscale& operator=(Grayscale&&) = default;

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
