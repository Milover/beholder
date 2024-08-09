/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	An image brightness and contrast normalization operation.

SourceFiles
	Morphology.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_MORPHOLOGY_OP_H
#define BEHOLDER_MORPHOLOGY_OP_H

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
                        Class Morphology Declaration
\*---------------------------------------------------------------------------*/

class Morphology
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

		//- Kernel
		int kernelType {0};	// cv::MORPH_RECT
		int kernelWidth {3};
		int kernelHeight {3};
		//- Morphology type
		int type {2};	// cv::MORPH_OPEN
		//- Number of morphology iterations
		int iterations {1};

	//- Constructors

		//- Default constructor
		Morphology() = default;

		//- Default constructor
		Morphology(int kTyp, int kW, int kH, int typ, int iter)
		:
			ProcessingOp(),
			kernelType {kTyp},
			kernelWidth {kW},
			kernelHeight {kH},
			type {typ},
			iterations {iter}
		{}

	//- Member functions

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
