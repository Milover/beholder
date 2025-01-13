// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

// A wrapper for the Tesseract-OCR API.
// For more info, see: https://tesseract-ocr.github.io/

#ifndef BEHOLDER_NEURAL_TESSERACT_H
#define BEHOLDER_NEURAL_TESSERACT_H

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "beholder/capi/Image.h"
#include "beholder/capi/Result.h"

namespace tesseract {
class TessBaseAPI;
}

namespace beholder {

class Tesseract {
private:
	// Deleter is a helper class for releasing Tesseract's underlying resources.
	struct Deleter {
		void operator()(tesseract::TessBaseAPI* t);
	};

	// The underlying Tesseract API.
	std::unique_ptr<tesseract::TessBaseAPI, Deleter> p_;
	// OCR results.
	std::vector<Result> res_;

public:
	// Configuration file paths.
	std::vector<std::string> configPaths;
	// Directory path of the model (trained data) file.
	std::string modelPath;
	// Model (trained data) name.
	std::string model;
	// Page segmentation mode.
	// NOTE: the value 6 is PSM_SINGLE_BLOCK, which Tesseract uses by default.
	int pageSegMode{6};	 // NOLINT(*-magic-numbers)
	// A map of settable variables.
	std::vector<std::pair<std::string, std::string>> variables{
		{"load_system_dawg", "0"}, {"load_freq_dawg", "0"}};

	// Default constructor
	Tesseract();

	Tesseract(const Tesseract&) = delete;
	Tesseract(Tesseract&&) = delete;

	~Tesseract();

	Tesseract& operator=(const Tesseract&) = delete;
	Tesseract& operator=(Tesseract&&) = delete;

	// Reset Tesseract and OCR results.
	void clear();

	// Run text detection and store the results.
	bool detectText();

	// Dump Tesseract variables to stdout.
	void dumpVariables() const;

	// Get No. dawgs.
	// Mostly for debugging and internal use.
	[[nodiscard]] int getNoDawgs() const;

	// Get a const reference to the OCR results.
	[[nodiscard]] const std::vector<Result>& getResults() const;

	// Initialize Tesseract.
	bool init();

	// Run text detection if necessary, then run text recognition
	// and store the results.
	//
	// Leading and trailing white space is trimmed from
	// the recognized text.
	bool recognizeText();

	// Set image for detection/recognition and clear all results.
	bool setImage(const Image& raw);
};

}  // namespace beholder

#endif	// BEHOLDER_NEURAL_TESSERACT_H
