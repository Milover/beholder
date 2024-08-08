/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <cstdio>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <tesseract/baseapi.h>

#include "Processor.h"
#include "Rectangle.h"
#include "Result.h"
#include "Tesseract.h"
#include "Utility.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
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
	res_.tags.clear();
	res_.boundingBoxes.clear();
}

// FIXME: should also grab other result data (eg. confidence)
bool Tesseract::detectText()
{
	res_.boundingBoxes.clear();

	std::unique_ptr<tesseract::PageIterator> iter {p_->AnalyseLayout()};
	if (!iter)
	{
		return false;
	}
	tesseract::PageIteratorLevel level {tesseract::RIL_TEXTLINE};

	// construct text boxes
	res_.boundingBoxes.reserve(5);		// FIXME: guesstimate
	do
	{
		Rectangle r;
		if (iter->BoundingBox(level, &r.left, &r.top, &r.right, &r.bottom))
		{
			res_.boundingBoxes.emplace_back(std::move(r));
		}
	} while(iter->Next(level));
	
	return !res_.boundingBoxes.empty();	// FIXME: we should ask TessBaseAPI if this succeeded
}

void Tesseract::dumpVariables() const
{
	p_->PrintVariables(stdout);
}

int Tesseract::getNoDawgs() const
{
	return p_->NumDawgs();
}

const Result& Tesseract::getResults() const
{
	return res_;
}

bool Tesseract::init()
{
	// we supply all variables to Init so everything gets loaded properly
	std::vector<std::string> vars;
	std::vector<std::string> vals;
	vars.reserve(variables.size());
	vals.reserve(variables.size());
	for (const auto& [key, val] : variables)
	{
		vars.emplace_back(key);
		vals.emplace_back(val);
	}
	auto path {vecStr2ChPtrArr(configPaths)};
	bool success
	{
		static_cast<bool>
		(
			p_->Init(
				modelPath.c_str(),
				model.c_str(),
				tesseract::OEM_LSTM_ONLY,
				path.get(),
				configPaths.size(),
				&vars,
				&vals,
				false
			)
		)
	};
	// Init returns 0 on success and -1 on failure, on the other hand,
	// int-to-bool conversion converts 0 to 'false' and everything else
	// to true, so we have to flip the result
	success = !success;

	// set page segmentation mode only if pageSegMode was explicitly set,
	// i.e. is different from the default
	if (pageSegMode != tesseract::PSM_SINGLE_BLOCK)
	{
		p_->SetPageSegMode(static_cast<tesseract::PageSegMode>(pageSegMode));
	}

	return success;
}

bool Tesseract::recognizeText()
{
	// TODO: would be nice to get text per bounding box
	char* c {p_->GetUTF8Text()};
	res_.tags.emplace_back(c);
	delete[] c;

	return !res_.tags.empty();	// FIXME: we should ask TessBaseAPI if this succeeded
}

bool Tesseract::detectAndRecognize()
{
	return detectText() && recognizeText();
}

void Tesseract::setImage(const Processor& ip, int bytesPerPixel)
{
	const cv::Mat& im {ip.getImage()};
	p_->SetImage(im.data, im.cols, im.rows, bytesPerPixel, im.step);
}

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// ************************************************************************* //
