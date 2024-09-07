/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	An object detector using the YOLOv8 model.
	A DNN-based object detector wrapper using the YOLOv8 model.
	For more info, see: https://github.com/ultralytics/ultralytics

SourceFiles
	YOLOv8Detector.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_YOLOv8_DETECTOR_H
#define BEHOLDER_YOLOv8_DETECTOR_H

#include "ObjDetector.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{


/*---------------------------------------------------------------------------*\
                      Class YOLOv8Detector Declaration
\*---------------------------------------------------------------------------*/

class YOLOv8Detector
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

	// Constructors

		//- Default constructor
		YOLOv8Detector()
		{
			scale = 1.0/255.0;
		}

		//- Disable copy constructor
		YOLOv8Detector(const YOLOv8Detector&) = default;

		//- Disable move constructor
		YOLOv8Detector(YOLOv8Detector&&) = default;

	//- Destructor
	virtual ~YOLOv8Detector() = default;

	// Member functions

	// Member operators

		//- Disable copy assignment
		YOLOv8Detector& operator=(const YOLOv8Detector&) = default;

		//- Disable move assignment
		YOLOv8Detector& operator=(YOLOv8Detector&&) = default;

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
