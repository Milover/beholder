/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	Postprocessing functions

SourceFiles
	Postprocess.cpp

\*---------------------------------------------------------------------------*/

#ifndef OCR_POSTPROCESS_H
#define OCR_POSTPROCESS_H

#include <vector>

#include <opencv2/opencv.hpp>	// TODO: break this out, we don't need all of it

#include "Config.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace ocr
{

// * * * * * * * * * * * * * * * * Functions * * * * * * * * * * * * * * * * //

//- Draw boxes onto an image
void drawBoxes
(
	cv::Mat& im,
	const std::vector<cv::Rect>& boxes,
	const Config& cfg
);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace ocr

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
