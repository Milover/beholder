// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#include "beholder/neural/Tesseract.h"

#include <tesseract/baseapi.h>

#include <cstdio>
#include <memory>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <string>
#include <utility>
#include <vector>

#include "beholder/capi/Image.h"
#include "beholder/capi/Rectangle.h"
#include "beholder/capi/Result.h"
#include "beholder/util/Constants.h"
#include "beholder/util/Utility.h"

namespace beholder {

void Tesseract::Deleter::operator()(tesseract::TessBaseAPI* t) {
	if (static_cast<bool>(t)) {
		t->End();
		delete t;
	}
}

// NOLINTNEXTLINE(*-use-equals-default): incomplete type; must be defined here
Tesseract::~Tesseract(){};

Tesseract::Tesseract() : p_{new tesseract::TessBaseAPI{}, Deleter{}} {}

void Tesseract::clear() {
	p_->Clear();
	res_.clear();
}

bool Tesseract::detectText() {
	res_.clear();
	res_.reserve(5);  // NOLINT(*-magic-numbers): guesstimate

	std::unique_ptr<tesseract::PageIterator> iter{p_->AnalyseLayout()};
	const tesseract::PageIteratorLevel level{tesseract::RIL_TEXTLINE};

	// construct text boxes
	bool ok{static_cast<bool>(iter)};
	while (ok) {
		Result r{};
		auto& b{r.box.ref()};
		if (iter->BoundingBox(level, &b.left, &b.top, &b.right, &b.bottom)) {
			res_.emplace_back(std::move(r));
		}
		ok = iter->Next(level);
	}
	// XXX: should we do further check to see if analysis succeeded, or is
	// checking iter enough?
	return !res_.empty();
}

void Tesseract::dumpVariables() const { p_->PrintVariables(stdout); }

int Tesseract::getNoDawgs() const { return p_->NumDawgs(); }

const std::vector<Result>& Tesseract::getResults() const { return res_; }

bool Tesseract::init() {
	// we supply all variables to Init so everything gets loaded properly
	std::vector<std::string> vars;
	std::vector<std::string> vals;
	vars.reserve(variables.size());
	vals.reserve(variables.size());
	for (const auto& [key, val] : variables) {
		vars.emplace_back(key);
		vals.emplace_back(val);
	}
	auto path{vecStr2ChPtrArr(configPaths)};
	bool success{static_cast<bool>(p_->Init(
		modelPath.c_str(), model.c_str(), tesseract::OEM_LSTM_ONLY, path.get(),
		static_cast<int>(configPaths.size()), &vars, &vals, false))};
	// Init returns 0 on success and -1 on failure, on the other hand,
	// int-to-bool conversion converts 0 to 'false' and everything else
	// to true, so we have to flip the result
	success = !success;

	// set page segmentation mode only if pageSegMode was explicitly set,
	// i.e. is different from the default
	if (pageSegMode != tesseract::PSM_SINGLE_BLOCK) {
		p_->SetPageSegMode(static_cast<tesseract::PageSegMode>(pageSegMode));
	}

	return success;
}

bool Tesseract::recognizeText() {
	// run detection first if necessary
	if (res_.empty() && !detectText()) {
		return false;
	}
	// XXX: does it make sense to do p_->SetRectangle(...), or should we
	// handle this with image processing?
	if (p_->Recognize(nullptr) != 0) {	// 0 means success, like EXIT_SUCCESS
		return false;
	}
	std::unique_ptr<tesseract::ResultIterator> iter{p_->GetIterator()};
	const tesseract::PageIteratorLevel level{tesseract::RIL_TEXTLINE};

	// get recognition results
	bool ok{static_cast<bool>(iter)};
	for (auto count{0UL}; ok; ++count) {
		// NOLINTNEXTLINE(*-c-arrays): needs to deallocate with delete[]
		const std::unique_ptr<char[]> ch{iter->GetUTF8Text(level)};
		if (ch) {
			res_[count].text = ch.get();
			trimWhiteLR(res_[count].text);
		}
		res_[count].confidence = iter->Confidence(level);

		ok = iter->Next(level);
	}
	// XXX: should we do further checks to see if recognition suceeded, or is
	// checking p_->Recognize() enough?
	return true;
}

bool Tesseract::setImage(const Image& raw) {
	res_.clear();

	const auto& ref{raw.cRef()};
	p_->SetImage(static_cast<unsigned char*>(ref.buffer), ref.cols, ref.rows,
				 static_cast<int>(ref.bitsPerPixel / cst::bits),
				 static_cast<int>(ref.step));
	return static_cast<bool>(p_->GetInputImage());
}

}  // namespace beholder
