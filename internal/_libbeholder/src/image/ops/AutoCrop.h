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

#include "AutoOrient.h"

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
	public AutoOrient
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

public:

	//- Constructors

		//- Default constructor
		AutoCrop() = default;

		//- Default constructor
		AutoCrop(int kS, float tH, float tW, float pad, double padV)
		:
			AutoOrient(kS, tH, tW, pad, padV)
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
