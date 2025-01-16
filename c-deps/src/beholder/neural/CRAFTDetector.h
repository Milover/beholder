// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

// A DNN-based text detector wrapper using the CRAFT model.
// For more info, see: https://github.com/clovaai/CRAFT-pytorch

#ifndef BEHOLDER_NEURAL_CRAFT_DETECTOR_H
#define BEHOLDER_NEURAL_CRAFT_DETECTOR_H

#include <array>

#include "beholder/neural/ObjDetector.h"

namespace cv {
class Mat;
}

namespace beholder {

class CRAFTDetector : public ObjDetector {
public:
	using Base = ObjDetector;

protected:
	// Extract inference results
	// TODO: extract confidences from NN output
	// TODO: clean up and optimize if possible
	void extract() override;

	// Store results
	// NOTE: has to happen after re-mapping from blob back to the image
	void store() override;

public:
	// NOLINTBEGIN(*-magic-numbers)

	float textThreshold{0.7};  // text confidence threshold
	float linkThreshold{0.4};  // link confidence threshold
	float lowText{0.4};		   // text low-bound schore

	// NOLINTEND(*-magic-numbers)

	// Default constructor.
	CRAFTDetector();

	CRAFTDetector(const CRAFTDetector&) = delete;
	CRAFTDetector(CRAFTDetector&&) = default;

	~CRAFTDetector() override = default;

	CRAFTDetector& operator=(const CRAFTDetector&) = delete;
	CRAFTDetector& operator=(CRAFTDetector&&) = default;
};

}  // namespace beholder

#endif	//  BEHOLDER_NEURAL_CRAFT_DETECTOR_H
