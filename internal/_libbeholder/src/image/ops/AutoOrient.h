/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	An automatic image orientation operation.

SourceFiles
	AutoOrient.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_AUTO_ORIENT_OP_H
#define BEHOLDER_AUTO_ORIENT_OP_H

#include <vector>

#include "ProcessingOp.h"

// * * * * * * * * * * * * * Forward Declarations  * * * * * * * * * * * * * //

namespace cv
{
	class Mat;
	class RotatedRect;
	template<typename T>
	class Point_;	// for Point2f, i.e. Point_<float>
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{

/*---------------------------------------------------------------------------*\
                        Class AutoOrient Declaration
\*---------------------------------------------------------------------------*/

class AutoOrient
:
	public ProcessingOp
{
protected:

	// Protected member functions

		//- Execute the processing operation
		virtual bool execute(const cv::Mat& in, cv::Mat& out) const override;

		//- Execute the processing operation
		virtual bool execute
		(
			const cv::Mat& in,
			cv::Mat& out,
			const std::vector<Result>&
		) const override;

		//- Execute implementation helper
		bool executeImpl
		(
			const cv::Mat& in,
			cv::Mat& out,
			cv::RotatedRect& box,
			cv::Point_<float>& center
		) const;

public:

	//- Public data

		//- Kernel size
		int kernelSize {50};
		//- Dimensions used for text box detection.
		//	Potential text boxes with smaller dimensions are discarded.
		float textHeight {50};
		float textWidth {50};
		//- Padding added to the cropped image
		float padding {10.0};
		//- Gradient kernel size
		//	XXX: is there a reason why we maed it 'const' originally?
		int gradientKernelSize {3};

	//- Constructors

		//- Default constructor
		AutoOrient() = default;

		//- Default constructor
		AutoOrient(int kS, float tH, float tW, float pad)
		:
			ProcessingOp(),
			kernelSize {kS},
			textHeight {tH},
			textWidth {tW},
			padding {pad}
		{}

		//- Default copy constructor
		AutoOrient(const AutoOrient&) = default;

		//- Default move constructor
		AutoOrient(AutoOrient&&) = default;

	//- Destructor
	virtual ~AutoOrient() = default;

	//- Member functions

	//- Member operators

		//- Default copy assignment
		AutoOrient& operator=(const AutoOrient&) = default;

		//- Default move assignment
		AutoOrient& operator=(AutoOrient&&) = default;

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

void findTextBox
(
	const cv::Mat& in,
	int kSize,
	float txtHeight,
	float txtWidth,
	float padding,
	int gKSize,
	cv::RotatedRect& returnValue
);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
