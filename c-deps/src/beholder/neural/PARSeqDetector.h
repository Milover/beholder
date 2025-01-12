// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

// A text recognition model using the PARSeq model.
// For more info, see: https://github.com/baudm/parseq

#ifndef BEHOLDER_NEURAL_PARSEQ_DETECTOR_H
#define BEHOLDER_NEURAL_PARSEQ_DETECTOR_H

#include <cstddef>

#include "neural/ObjDetector.h"

namespace beholder {

class PARSeqDetector : public ObjDetector {
public:
	using Base = ObjDetector;

protected:
	// Extract inference results.
	void extract() override;

	// Store results.
	// NOTE: no-op, everything stored during extraction.
	void store() override{};

public:
	// The max length of the recognized list of characters, i.e.
	// the number of output positions.
	//
	// For more info, see:
	// https://github.com/baudm/parseq/tree/main?tab=readme-ov-file#method-tldr
	inline static constexpr int nPos{26};

	// The character set recognized by the model.
	std::string charset{
		R"(0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!"#$%&'()*+,-./:;<=>?@[\]^_`{|}~)"};

	// Default constructor
	PARSeqDetector();

	PARSeqDetector(const PARSeqDetector&) = delete;
	PARSeqDetector(PARSeqDetector&&) = default;

	~PARSeqDetector() override = default;

	PARSeqDetector& operator=(const PARSeqDetector&) = delete;
	PARSeqDetector& operator=(PARSeqDetector&&) = default;
};

}  // namespace beholder

#endif	// BEHOLDER_NEURAL_PARSEQ_DETECTOR_H
