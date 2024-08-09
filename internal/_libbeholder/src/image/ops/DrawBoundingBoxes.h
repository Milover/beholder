/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	An image brightness and contrast normalization operation.

SourceFiles
	DrawBoundingBoxes.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_DRAW_BOUNDING_BOXES_OP_H
#define BEHOLDER_DRAW_BOUNDING_BOXES_OP_H

#include <array>
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
                    Class DrawBoundingBoxes Declaration
\*---------------------------------------------------------------------------*/

class DrawBoundingBoxes
:
	public ProcessingOp
{
protected:

	// Protected member functions

		//- Execute the processing operation
		bool execute(const cv::Mat&, cv::Mat&) const override;

		//- Execute the processing operation
		bool execute
		(
			const cv::Mat&,
			cv::Mat& out,
			const std::vector<Result>& res
		) const override;

public:

	using Color = std::array<float, 4>;

	//- Public data

		//- Bounding box border color
		Color color {0.0, 0.0, 0.0, 0.0};

		//- Bounding box border thickness
		int thickness {2};

	//- Constructors

		//- Default constructor
		DrawBoundingBoxes() = default;

		//- Default constructor
		DrawBoundingBoxes(const Color& c, int t)
		:
			ProcessingOp(),
			color {c},
			thickness {t}
		{}

	//- Member functions

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
