/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	An image histogram equalization operation

SourceFiles
	EqualizeHistogram.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_EQUALIZE_HISTOGRAM_OP_H
#define BEHOLDER_EQUALIZE_HISTOGRAM_OP_H

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
                       Class EqualizeHistogram Declaration
\*---------------------------------------------------------------------------*/

class EqualizeHistogram
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

	//- Constructors

		//- Default constructor
		EqualizeHistogram() = default;

		//- Default copy constructor
		EqualizeHistogram(const EqualizeHistogram&) = default;

		//- Default move constructor
		EqualizeHistogram(EqualizeHistogram&&) = default;

	//- Destructor
	virtual ~EqualizeHistogram() = default;

	//- Member functions

	//- Member operators

		//- Default copy assignment
		EqualizeHistogram& operator=(const EqualizeHistogram&) = default;

		//- Default move assignment
		EqualizeHistogram& operator=(EqualizeHistogram&&) = default;

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
