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

#include "ImageProcessor.h"
#include "OcrResults.h"
#include "Rectangle.h"
#include "Tesseract.h"
#include "Utility.h"

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

// FIXME: should clear results
void Tesseract::clear()
{
	p_->Clear();
	res_.text.clear();
	res_.textBoxes.clear();
}


// FIXME: should also grab other result data (eg. certanties)
bool Tesseract::detectText()
{
	res_.textBoxes.clear();

	std::unique_ptr<tesseract::PageIterator> iter {p_->AnalyseLayout()};
	if (!iter)
	{
		return false;
	}
	tesseract::PageIteratorLevel level {tesseract::RIL_TEXTLINE};

	// construct text boxes
	res_.textBoxes.reserve(5);		// FIXME: guesstimate
	do
	{
		Rectangle r;
		if (iter->BoundingBox(level, &r.left, &r.top, &r.right, &r.bottom))
		{
			res_.textBoxes.emplace_back(std::move(r));
		}
	} while(iter->Next(level));
	
	return !res_.textBoxes.empty();	// FIXME: we should ask TessBaseAPI if this succeeded
}


const OcrResults& Tesseract::getResults() const
{
	return res_;
}


bool Tesseract::init()
{
	auto p {vecStr2ChPtrArr(configPaths)};
	bool success
	{
		static_cast<bool>
		(
			p_->Init(
				modelPath.c_str(),
				model.c_str(),
				tesseract::OEM_LSTM_ONLY,
				p.get(),
				configPaths.size(),
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

	p_->SetPageSegMode(static_cast<tesseract::PageSegMode>(pageSegMode));
	for (const auto& [key, val] : variables)
	{
		success = success && p_->SetVariable(key.c_str(), val.c_str());
	}

	return success;
}


bool Tesseract::recognizeText()
{
	char* c {p_->GetUTF8Text()};
	res_.text = c;
	delete[] c;

	return !res_.text.empty();	// FIXME: we should ask TessBaseAPI if this succeeded
}


bool Tesseract::detectAndRecognize()
{
	return detectText() && recognizeText();
}


void Tesseract::setImage(const ImageProcessor& ip, int bytesPerPixel)
{
	const cv::Mat& im {ip.getImage()};
	p_->SetImage(im.data, im.cols, im.rows, bytesPerPixel, im.step);
}

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace ocr

// ************************************************************************* //
