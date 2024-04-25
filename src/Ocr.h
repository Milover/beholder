/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	Text detection and recognition functions.

SourceFiles
	Ocr.cpp

\*---------------------------------------------------------------------------*/

#ifndef OCR_OCR_H
#define OCR_OCR_H

#include <memory>
#include <optional>
#include <vector>

#include <tesseract/baseapi.h>
#include <opencv2/opencv.hpp>

#include "Config.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace ocr
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace detail
{

// RAII helper class for the tesseract::TessBaseAPI
struct  TBADeleter
{
	void operator()(tesseract::TessBaseAPI* p)
	{
		p->End();
		delete p;
	}
};

} // End namespace detail

// * * * * * * * * * * * * * * * * Functions * * * * * * * * * * * * * * * * //

// Setup Tesseract
std::unique_ptr<tesseract::TessBaseAPI, detail::TBADeleter>
initialize(const Config& cfg);

// Detect text and generate text boxes
std::optional<std::vector<cv::Rect>> detectText
(
	const std::unique_ptr<tesseract::TessBaseAPI, detail::TBADeleter>& api
);

// Draw text boxes onto an image
void drawTextBoxes
(
	cv::Mat& im,
	const std::vector<cv::Rect>& rects,
	const Config& cfg
);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace ocr

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
