/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	An image processing operation

SourceFiles
	ProcessingOp.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_PROCESSING_OP_H
#define BEHOLDER_PROCESSING_OP_H

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <functional>

#include "Result.h"

// * * * * * * * * * * * * * Forward Declarations  * * * * * * * * * * * * * //

namespace cv
{
	class Mat;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{

/*---------------------------------------------------------------------------*\
                       Class ProcessingOp Declaration
\*---------------------------------------------------------------------------*/

class ProcessingOp
{
protected:

	// Protected constructors

		//- Default constructor
		ProcessingOp() = default;

	// Protected member functions

		//- Execute the processing operation
		virtual bool execute(const cv::Mat& in, cv::Mat& out) const = 0;

		//- Execute the processing operation
		virtual bool execute
		(
			const cv::Mat& in,
			cv::Mat& out,
			const Result& res
		) const = 0;

public:

	using OpPtr = std::unique_ptr<ProcessingOp>;
	using Selector = std::function<OpPtr()>;
	using SelectorEntry = std::pair<std::string, Selector>;
	using SelectorTable = std::vector<SelectorEntry>;

	//- Public data

		//- A table containing the names and static constructors of
		//	all implemented processing operations
		static const SelectorTable OpTable;

	// Constructors

		//- Default copy constructor
		ProcessingOp(const ProcessingOp&) = default;

		//- Default move constructor
		ProcessingOp(ProcessingOp&&) = default;

	//- Destructor
	virtual ~ProcessingOp() = default;

	//- Member functions

	//- Member operators

		//- Call operator
		bool operator()(const cv::Mat& in, cv::Mat& out) const;

		//- Call operator
		bool operator()
		(
			const cv::Mat& in,
			cv::Mat& out,
			const Result& res
		) const;

		//- Default copy assignment
		ProcessingOp& operator=(const ProcessingOp&) = default;

		//- Default move assignment
		ProcessingOp& operator=(ProcessingOp&&) = default;

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
