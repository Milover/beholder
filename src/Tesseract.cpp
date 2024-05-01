/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <memory>
#include <string>
#include <vector>

#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>

#include "Config.h"
#include "Utility.h"

#include "ImageProcessor.h"
#include "Tesseract.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace ocr
{

// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * * //

Tesseract::Tesseract()
:
	p_ {new Tesseract::TBA{}}
{}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Tesseract::~Tesseract()
{
	if (p_)
	{
		p_->End();
		delete p_;
		p_ = nullptr;
	}
}

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

void Tesseract::clear()
{
	p_->Clear();
}


std::vector<cv::Rect> Tesseract::detectText()
{
	std::vector<cv::Rect> rects;

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


bool Tesseract::init(const Config& cfg)
{
	bool success
	{
		static_cast<bool>
		(
			p_->Init(
				cfg.modelPath,
				cfg.model,
				tesseract::OEM_LSTM_ONLY,
				cfg.configPaths,
				cfg.nConfigPaths,
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
