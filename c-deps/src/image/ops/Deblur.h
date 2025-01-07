/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	An out of focus image deblurring operation.

	For more info, see:
	https://docs.opencv.org/4.10.0/de/d3c/tutorial_out_of_focus_deblur_filter.html

SourceFiles
	Deblur.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_DEBLUR_OP_H
#define BEHOLDER_DEBLUR_OP_H

#include <vector>

#include "ProcessingOp.h"

// * * * * * * * * * * * * * Forward Declarations  * * * * * * * * * * * * * //

namespace cv
{
	class Mat;
	template<typename T>
	class Size_;	// for Size == Size2i == Size_<int>
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{

/*---------------------------------------------------------------------------*\
                        Class Deblur Declaration
\*---------------------------------------------------------------------------*/

class Deblur
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

public:

	//- Public data

		//- Deblur radius
		int radius {5};
		//- Signal to noise ratio
		int snr {100};

	//- Constructors

		//- Default constructor
		Deblur(int rad, int snrRatio)
		:
			radius {rad},
			snr {snrRatio}
		{}

		//- Default constructor
		Deblur() = default;

		//- Default copy constructor
		Deblur(const Deblur&) = default;

		//- Default move constructor
		Deblur(Deblur&&) = default;

	//- Destructor
	virtual ~Deblur() = default;

	//- Member functions

	//- Member operators

		//- Default copy assignment
		Deblur& operator=(const Deblur&) = default;

		//- Default move assignment
		Deblur& operator=(Deblur&&) = default;

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

void computePSF(cv::Mat& out, const cv::Size_<int>& filterSize, int R);

void fftShift(const cv::Mat& in, cv::Mat& out);

void filter2Dfreq(const cv::Mat& in, cv::Mat& out, const cv::Mat& H);

void computeWeinerFilter(const cv::Mat& in, cv::Mat& out, double nsr);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
