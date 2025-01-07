/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	An image brightness and contrast normalization operation.

SourceFiles
	DrawLabels.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_DRAW_LABELS_OP_H
#define BEHOLDER_DRAW_LABELS_OP_H

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
                    Class DrawLabels Declaration
\*---------------------------------------------------------------------------*/

class DrawLabels
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

		//- Font color
		Color color {0.0, 0.0, 0.0, 0.0};
		//- Font scale
		double fontScale {1};
		//- Line thickness
		int thickness {2};

	//- Constructors

		//- Default constructor
		DrawLabels() = default;

		//- Default constructor
		DrawLabels(const Color& c, double s, int t)
		:
			ProcessingOp(),
			color {c},
			fontScale {s},
			thickness {t}
		{}

		//- Default copy constructor
		DrawLabels(const DrawLabels&) = default;

		//- Default move constructor
		DrawLabels(DrawLabels&&) = default;

	//- Destructor
	virtual ~DrawLabels() = default;

	//- Member functions

	//- Member operators

		//- Default copy assignment
		DrawLabels& operator=(const DrawLabels&) = default;

		//- Default move assignment
		DrawLabels& operator=(DrawLabels&&) = default;

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
