/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A DNN-based text detector wrapper using the EAST model.
	For more info, see: https://github.com/argman/EAST

SourceFiles
	EASTDetector.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_EAST_DETECTOR_H
#define BEHOLDER_EAST_DETECTOR_H

#include <array>

#include "ObjDetector.h"

// * * * * * * * * * * * * * Forward Declarations  * * * * * * * * * * * * * //

namespace cv
{
	class Mat;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{

/*---------------------------------------------------------------------------*\
                      Class EASTDetector Declaration
\*---------------------------------------------------------------------------*/

class EASTDetector
:
	public ObjDetector
{
public:

	using Base = ObjDetector;

protected:

	// Protected member functions

		//- Extract inference results
		virtual void extract() override;

public:

	// Public data

	// Constructors

		//- Default constructor
		EASTDetector()
		{
			// For more info, see:
			// https://docs.opencv.org/4.10.0/d4/d43/tutorial_dnn_text_spotting.html
			mean = Base::Vec3<> {123.68, 116.78, 103.94};
		}

		//- Disable copy constructor
		EASTDetector(const EASTDetector&) = default;

		//- Disable move constructor
		EASTDetector(EASTDetector&&) = default;


	//- Destructor
	virtual ~EASTDetector() = default;

	// Member functions

	// Member operators

		//- Disable copy assignment
		EASTDetector& operator=(const EASTDetector&) = default;

		//- Disable move assignment
		EASTDetector& operator=(EASTDetector&&) = default;

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
