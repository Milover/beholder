/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A DNN-based text detector wrapper using the CRAFT model.
	For more info, see: https://github.com/clovaai/CRAFT-pytorch

SourceFiles
	CRAFTDetector.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_CRAFT_DETECTOR_H
#define BEHOLDER_CRAFT_DETECTOR_H

#include <array>

#include "neural/ObjDetector.h"

// * * * * * * * * * * * * * Forward Declarations  * * * * * * * * * * * * * //

namespace cv
{
	class Mat;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{

/*---------------------------------------------------------------------------*\
                      Class CRAFTDetector Declaration
\*---------------------------------------------------------------------------*/

class CRAFTDetector
:
	public ObjDetector
{
public:

	using Base = ObjDetector;

protected:

	// Protected member functions

		//- Extract inference results
		//	TODO: extract confidences from NN output
		//	TODO: clean up and optimize if possible
		virtual void extract() override;

		//- Store results
		//	NOTE: has to happen after re-mapping from blob back to the image
		virtual void store() override;

public:

	// Public data

		//- Text confidence threshold
		float textThreshold {0.7};
		//- Link confidence threshold
		float linkThreshold {0.4};
		//- Text low-bound schore
		float lowText {0.4};

	// Constructors

		//- Default constructor
		CRAFTDetector()
		{
			// For more info, see:
			// https://github.com/clovaai/CRAFT-pytorch/blob/e332dd8b718e291f51b66ff8f9ef2c98ee4474c8/imgproc.py#L20
			scale = Base::Vec3<>
			{
				1.0 / (0.229*255.0),
				1.0 / (0.224*255.0),
				1.0 / (0.225*225.0)
			};
			mean = Base::Vec3<> {0.485*255.0, 0.456*255.0, 0.406*255.0};
		}

		//- Disable copy constructor
		CRAFTDetector(const CRAFTDetector&) = default;

		//- Disable move constructor
		CRAFTDetector(CRAFTDetector&&) = default;


	//- Destructor
	virtual ~CRAFTDetector() = default;

	// Member functions

	// Member operators

		//- Disable copy assignment
		CRAFTDetector& operator=(const CRAFTDetector&) = default;

		//- Disable move assignment
		CRAFTDetector& operator=(CRAFTDetector&&) = default;

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
