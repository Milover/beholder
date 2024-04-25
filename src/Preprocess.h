/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	Image preprocessing functions.

SourceFiles
	Preprocess.cpp

\*---------------------------------------------------------------------------*/

#ifndef OCR_PREPROCESS_H
#define OCR_PREPROCESS_H

#include <opencv2/opencv.hpp>	// TODO: break this out, we don't need all of it

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace ocr
{

// * * * * * * * * * * * * * * * * Functions * * * * * * * * * * * * * * * * //

// Normalize brightness and contrast
//
// TODO: this needs to be refactored
void normalize(cv::Mat& im, float clipPct = 0.5);


// Prepare an image for OCR
//
// TODO: this should probably take a config, so we can define the
// preprocessing pipeline without recompilation
void preprocess(cv::Mat& im);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace ocr

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
