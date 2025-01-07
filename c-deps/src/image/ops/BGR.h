/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A grayscale-image-to-image conversion operation.

SourceFiles
	BGR.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_BGR_OP_H
#define BEHOLDER_BGR_OP_H

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
                            Class BGR Declaration
\*---------------------------------------------------------------------------*/

class BGR
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
		BGR() = default;

		//- Default copy constructor
		BGR(const BGR&) = default;

		//- Default move constructor
		BGR(BGR&&) = default;

	//- Destructor
	virtual ~BGR() = default;

	//- Member functions

	//- Member operators

		//- Default copy assignment
		BGR& operator=(const BGR&) = default;

		//- Default move assignment
		BGR& operator=(BGR&&) = default;

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
