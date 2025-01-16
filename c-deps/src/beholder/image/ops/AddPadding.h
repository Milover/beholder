// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BEHOLDER_IMAGE_OPS_ADD_PADDING_H
#define BEHOLDER_IMAGE_OPS_ADD_PADDING_H

#include <vector>

#include "beholder/image/ProcessingOp.h"
#include "beholder/util/Constants.h"

namespace cv {
class Mat;
}

namespace beholder {

// AddPadding is an operation which adds uniform padding around
// the border of an image.
class AddPadding : public ProcessingOp {
protected:
	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out) const override;

	// Execute the processing operation.
	bool execute(const cv::Mat& in, cv::Mat& out,
				 const std::vector<Result>& res) const override;

public:
	// NOLINTBEGIN(*-magic-numbers)

	int padding{10};				// number of pixel layers of padding
	double padValue{cst::max8bit};	// pixel value to use for padding

	// NOLINTEND(*-magic-numbers)

	// Default constructor.
	AddPadding() = default;

	// Default constructor.
	// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
	AddPadding(int pad, double pv) : padding{pad}, padValue{pv} {}

	AddPadding(const AddPadding&) = default;
	AddPadding(AddPadding&&) = default;

	~AddPadding() override = default;

	AddPadding& operator=(const AddPadding&) = default;
	AddPadding& operator=(AddPadding&&) = default;
};

}  // namespace beholder

#endif	// BEHOLDER_IMAGE_OPS_ADD_PADDING_H
