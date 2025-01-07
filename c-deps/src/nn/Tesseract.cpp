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

#include "RawImage.h"
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
	res_.clear();
}

bool Tesseract::detectText()
{
	res_.clear();

	std::unique_ptr<tesseract::PageIterator> iter {p_->AnalyseLayout()};
	if (!iter)
	{
		return false;
	}
	tesseract::PageIteratorLevel level {tesseract::RIL_TEXTLINE};

	// construct text boxes
	res_.reserve(5);		// FIXME: guesstimate
	do
	{
		Result r {};
		auto& b {r.box.ref()};
		if (iter->BoundingBox(level, &b.left, &b.top, &b.right, &b.bottom))
		{
			res_.emplace_back(std::move(r));
		}
	} while(iter->Next(level));
	// XXX: should we do further check to see if analysis succeeded, or is
	// checking iter enough?
	return !res_.empty();
}

void Tesseract::dumpVariables() const
{
	p_->PrintVariables(stdout);
}

int Tesseract::getNoDawgs() const
{
	return p_->NumDawgs();
}

const std::vector<Result>& Tesseract::getResults() const
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
	// run detection first if necessary
	if (res_.empty() && !detectText())
	{
		return false;
	}
	// XXX: does it make sense to do p_->SetRectangle(...), or should we
	// handle this with image processing?
	if (p_->Recognize(nullptr) != 0)
	{
		return false;
	}
	std::unique_ptr<tesseract::ResultIterator> iter {p_->GetIterator()};
	tesseract::PageIteratorLevel level {tesseract::RIL_TEXTLINE};

	// get recognition results
	auto count {0ul};
	do
	{
		if (char* ch {iter->GetUTF8Text(level)}; ch)
		{
			res_[count].text = ch;
			trimWhiteLR(res_[count].text);
			delete[] ch;
		}

		res_[count].confidence = iter->Confidence(level);

		++count;
	} while(iter->Next(level));
	// XXX: should we do further checks to see if recognition suceeded, or is
	// checking p_->Recognize() enough?
	return true;
}

bool Tesseract::setImage(const RawImage& raw)
{
	res_.clear();

	const auto& ref {raw.cRef()};
	p_->SetImage
	(
		static_cast<unsigned char*>(ref.buffer),
		ref.cols,
		ref.rows,
		static_cast<int>(ref.bitsPerPixel / 8),	// bits to bytes
		static_cast<int>(ref.step)
	);
	return static_cast<bool>(p_->GetInputImage());
}

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// ************************************************************************* //
