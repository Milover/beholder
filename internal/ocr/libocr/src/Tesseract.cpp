/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <tesseract/baseapi.h>

#include "Config.h"
#include "Utility.h"

#include "ImageProcessor.h"
#include "Rectangle.h"
#include "Tesseract.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace ocr
{

// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * * //

Tesseract::Tesseract()
:
	p_ {new tesseract::TessBaseAPI{}}
{}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Tesseract::~Tesseract()
{
	p_->End();
	delete p_;
}

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

void Tesseract::clear()
{
	p_->Clear();
}


std::vector<Rectangle> Tesseract::detectText()
{
	std::vector<Rectangle> rects;

	std::unique_ptr<tesseract::PageIterator> iter {p_->AnalyseLayout()};
	if (!iter)
	{
		return rects;
	}
	tesseract::PageIteratorLevel level {tesseract::RIL_TEXTLINE};

	// construct text boxes
	rects.reserve(5);		// FIXME: guesstimate
	do
	{
		Rectangle r;
		if (iter->BoundingBox(level, &r.left, &r.top, &r.right, &r.bottom))
		{
			rects.emplace_back(std::move(r));
		}
	} while(iter->Next(level));

	return rects;
}


bool Tesseract::init(const Config& cfg)
{
	auto p {vecStr2ChPtrArr(cfg.configPaths)};
	bool success
	{
		static_cast<bool>
		(
			p_->Init(
				cfg.modelPath.c_str(),
				cfg.model.c_str(),
				tesseract::OEM_LSTM_ONLY,
				p.get(),
				cfg.configPaths.size(),
				nullptr,
				nullptr,
				false
			)
		)
	};
	// Init returns 0 on success and -1 on failure, on the other hand,
	// int-to-bool conversion converts 0 to 'false' and everything else
	// to true, so we have to flip the result
	success = !success;

	// NOTE: would be nice if we could get it to a single line/word
	p_->SetPageSegMode(tesseract::PSM_SINGLE_BLOCK);
	success = success && p_->SetVariable("load_system_dawg", "0");
	success = success && p_->SetVariable("load_freq_dawg", "0");
//	success = success && p_->SetVariable("classify_bln_numeric_mode", "1");
//	success = success && p_->SetVariable("tessedit_char_whitelist", ".,:;0123456789");

	return success;
}


char* Tesseract::recognizeText()
{
	return p_->GetUTF8Text();
}


std::string Tesseract::recognizeTextStr()
{
	char* c {p_->GetUTF8Text()};
	std::string s {c};
	delete[] c;
	return s;
}


void Tesseract::setImage(const ImageProcessor& ip, int bytesPerPixel)
{
	const cv::Mat& im {ip.getImage()};
	p_->SetImage(im.data, im.cols, im.rows, bytesPerPixel, im.step);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace ocr

// ************************************************************************* //
