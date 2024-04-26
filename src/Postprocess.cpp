/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <vector>

#include <opencv2/opencv.hpp>	// TODO: break this out, we don't need all of it

#include "Config.h"
#include "Postprocess.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

namespace ocr
{

// * * * * * * * * * * * * * * * * Functions * * * * * * * * * * * * * * * * //

void drawBoxes
(
	cv::Mat& im,
	const std::vector<cv::Rect>& boxes,
	const Config& cfg
)
{
	for (const auto& b : boxes)
	{
		cv::rectangle(im, b, cfg.textBoxColor, cfg.textBoxThickness);
	}
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace ocr

// ************************************************************************* //
