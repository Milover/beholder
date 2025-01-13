// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#ifndef BEHOLDER_IMAGE_OPS_MORPHOLOGY_H
#define BEHOLDER_IMAGE_OPS_MORPHOLOGY_H

#include <vector>

#include "beholder/image/ProcessingOp.h"

namespace cv {
class Mat;
}

namespace beholder {

class Morphology : public ProcessingOp {
protected:
	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out) const override;

	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out,
				 const std::vector<Result>& res) const override;

public:
	// Supported types of morphological operations.
	// For more info, see:
	// https://docs.opencv.org/4.10.0/d4/d86/group__imgproc__filter.html#ga67493776e3ad1a3df63883829375201f
	enum class Type {
		Erode,
		Dilate,
		Open,
		Close,
		Gradient,
		TopHat,
		BlackHat,
		HitMiss
	};
	// Supported kernel shapes for morphological operations.
	// For more info, see:
	// https://docs.opencv.org/4.10.0/d4/d86/group__imgproc__filter.html#gac2db39b56866583a95a5680313c314ad
	enum class Shape { Box, Cross, Ellipse };

	Type type{Type::Open};	  // morphology type
	Shape shape{Shape::Box};  // kernel shape type
	int width{3};			  // kernel width
	int height{3};			  // kernel width
	int iterations{1};		  // number of morphology iterations

	// Default constructor.
	Morphology() = default;

	// Default constructor.
	// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
	Morphology(Shape s, int w, int h, Type typ, int iter)
		: type{typ}, shape{s}, width{w}, height{h}, iterations{iter} {}

	Morphology(const Morphology&) = default;
	Morphology(Morphology&&) = default;

	~Morphology() override = default;

	Morphology& operator=(const Morphology&) = default;
	Morphology& operator=(Morphology&&) = default;
};

}  // namespace beholder

#endif	// BEHOLDER_IMAGE_OPS_MORPHOLOGY_H
