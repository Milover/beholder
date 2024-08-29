/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A helper class for holding temporary structures created during
	object detection.

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_OBJ_DETECTOR_BUFFERS_H
#define BEHOLDER_OBJ_DETECTOR_BUFFERS_H

#include <vector>

#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{
namespace internal
{

/*---------------------------------------------------------------------------*\
                     Class ObjDetectorBuffers Declaration
\*---------------------------------------------------------------------------*/

class ObjDetectorBuffers
{
public:

	// Public data

	//- Temporaries used during ObjDetector::detect() and ObjDetector::extract()
    cv::Mat blob;
    std::vector<cv::Mat> outs;			// forward results
	// XXX: could also use RotatedRects
	std::vector<cv::Rect> tBoxes;		// unfiltered blob boxes
	std::vector<int> tClassIDs;			// unfiltered class IDs
	std::vector<float> tConfidences;	// unfiltered confidences
	std::vector<int> tNMSIDs;			// IDs used during NMS filtering

	// Member functions

		//- Clear buffers, but keep allocated memory.
		void clear()
		{
			outs.clear();
			tBoxes.clear();
			tClassIDs.clear();
			tConfidences.clear();
			tNMSIDs.clear();
		}
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace internal
} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
