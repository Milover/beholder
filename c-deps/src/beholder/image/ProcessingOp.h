// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

// An image processing operation class definition.

#ifndef BEHOLDER_IMAGE_PROCESSING_OP_H
#define BEHOLDER_IMAGE_PROCESSING_OP_H

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "beholder/capi/Result.h"

namespace cv {
class Mat;
}

namespace beholder {

class ProcessingOp {
protected:
	// Default constructor
	ProcessingOp() = default;

	// Execute the (pre-)processing operation.
	virtual bool execute(const cv::Mat& in, cv::Mat& out) const = 0;

	// Execute the (post-)processing operation.
	virtual bool execute(const cv::Mat& in, cv::Mat& out,
						 const std::vector<Result>& res) const = 0;

public:
	using OpPtr = std::unique_ptr<ProcessingOp>;
	using Selector = std::function<OpPtr()>;
	using SelectorEntry = std::pair<std::string, Selector>;
	using SelectorTable = std::vector<SelectorEntry>;

	// A table containing the names and static constructors of
	// all implemented processing operations
	static const SelectorTable OpTable;

	ProcessingOp(const ProcessingOp&) = default;
	ProcessingOp(ProcessingOp&&) = default;

	virtual ~ProcessingOp() = default;

	ProcessingOp& operator=(const ProcessingOp&) = default;
	ProcessingOp& operator=(ProcessingOp&&) = default;

	// Execute a processing operation which does not require pipeline results,
	// usually a pre-processing operation.
	//
	// Operations which don't require results usually implement/use this
	// version of the call operator.
	// Operations which require results (eg. rotating an image) usually
	// fail and return false outright.
	//
	// TODO: this should be removed and merged with the call operator below.
	// Results should get ignored when not necessary, or defaulted when they
	// are but are not provided.
	bool operator()(const cv::Mat& in, cv::Mat& out) const;

	// Execute a processing operation which does requires pipeline results,
	// usually a post-processing operation.
	//
	// Operations which require results (eg. drawing rectangles on an image)
	// usually only define/use this version of the call operator.
	// Operations which don't require results typically just ignore the results
	// and function as usual.
	bool operator()(const cv::Mat& in, cv::Mat& out,
					const std::vector<Result>& res) const;
};

}  // namespace beholder

#endif	// BEHOLDER_IMAGE_PROCESSING_OP_H
