// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

// A DNN-based text detector wrapper using the EAST model.
// For more info, see: https://github.com/argman/EAST

#ifndef BEHOLDER_NEURAL_EAST_DETECTOR_H
#define BEHOLDER_NEURAL_EAST_DETECTOR_H

#include <array>

#include "neural/ObjDetector.h"

namespace cv {
class Mat;
}

namespace beholder {

class EASTDetector : public ObjDetector {
public:
	using Base = ObjDetector;

protected:
	// Extract inference results.
	void extract() override;

	// Filter (NMS) and store results.
	void store() override;

public:
	// Default constructor.
	EASTDetector();

	EASTDetector(const EASTDetector&) = delete;
	EASTDetector(EASTDetector&&) = default;

	~EASTDetector() override = default;

	EASTDetector& operator=(const EASTDetector&) = delete;
	EASTDetector& operator=(EASTDetector&&) = default;
};

}  // namespace beholder

#endif	// BEHOLDER_NEURAL_EAST_DETECTOR_H
