/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <tesseract/baseapi.h>
#include <opencv2/opencv.hpp>

#include "Config.h"
#include "Ocr.h"
#include "Utility.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace ocr
{

// * * * * * * * * * * * * * * * * Functions * * * * * * * * * * * * * * * * //

// Setup Tesseract
std::unique_ptr<tesseract::TessBaseAPI, detail::TBADeleter>
initialize(const Config& cfg)
{
	using tba = tesseract::TessBaseAPI;
	auto api = std::unique_ptr<tba, detail::TBADeleter>
	(
		new tba{}, detail::TBADeleter{}
	);

	auto configs {vectorStrings2UniqueCharPtr(cfg.configPaths)};
	for (auto i {0ul}; i < cfg.configPaths.size(); ++i)
	{
		configs[i] = const_cast<char*>(cfg.configPaths[i].c_str());
	}
	if (
		api->Init(
			cfg.modelPath.c_str(),
			cfg.model.c_str(),
			tesseract::OEM_LSTM_ONLY,
			configs.get(),
			cfg.configPaths.size(),
			nullptr,
			nullptr,
			false
		)
	)
	{
		api.reset();
		return api;
	}
	// NOTE: would be nice if we could get it to a single line/word
	api->SetPageSegMode(tesseract::PSM_SINGLE_BLOCK);
	api->SetVariable("load_system_dawg", "0");
	api->SetVariable("load_freq_dawg", "0");
//	api->SetVariable("classify_bln_numeric_mode", "1");
//	api->SetVariable("tessedit_char_whitelist", ".,:;0123456789");

	return api;
}

// Detect text and generate text boxes
std::optional<std::vector<cv::Rect>> detectText
(
	const std::unique_ptr<tesseract::TessBaseAPI, detail::TBADeleter>& api
)
{
	std::unique_ptr<tesseract::PageIterator> iter {api->AnalyseLayout()};
	if (!iter)
	{
		return std::nullopt;
	}
	tesseract::PageIteratorLevel level {tesseract::RIL_TEXTLINE};

	// construct text boxes
	std::vector<cv::Rect> rects;
	rects.reserve(5);		// FIXME: guesstimate
	do
	{
		int left, top, right, bottom;
		if (iter->BoundingBox(level, &left, &top, &right, &bottom))
		{
			rects.emplace_back(
				cv::Point{left, top}, cv::Point{right, bottom}
			);
		}
	} while(iter->Next(level));

	return rects;
}

// Draw text boxes onto an image
void drawTextBoxes
(
	cv::Mat& im,
	const std::vector<cv::Rect>& rects,
	const Config& cfg
)
{
	for (const auto& r : rects)
	{
		cv::rectangle(im, r, cfg.textBoxColor, cfg.textBoxThickness);
	}
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace ocr

// ************************************************************************* //
