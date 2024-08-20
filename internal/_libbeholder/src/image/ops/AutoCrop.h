/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	An automatic image cropping operation.

SourceFiles
	AutoCrop.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_AUTO_CROP_OP_H
#define BEHOLDER_AUTO_CROP_OP_H

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
                        Class AutoCrop Declaration
\*---------------------------------------------------------------------------*/

class AutoCrop
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
		AutoCrop() = default;

		//- Default constructor
		AutoCrop(int kS, float tH, float tW, float pad)
		:
			ProcessingOp(),
			kernelSize {kS},
			textHeight {tH},
			textWidth {tW},
			padding {pad}
		{}

		//- Default copy constructor
		AutoCrop(const AutoCrop&) = default;

		//- Default move constructor
		AutoCrop(AutoCrop&&) = default;

	//- Destructor
	virtual ~AutoCrop() = default;

	//- Member functions

	//- Member operators

		//- Default copy assignment
		AutoCrop& operator=(const AutoCrop&) = default;

		//- Default move assignment
		AutoCrop& operator=(AutoCrop&&) = default;

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
