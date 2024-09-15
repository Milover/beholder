/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A text recognition model using the PARSeq model.
	For more info, see: https://github.com/baudm/parseq

SourceFiles
	PARSeqDetector.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_PARSEQ_DETECTOR_H
#define BEHOLDER_PARSEQ_DETECTOR_H

#include <cstddef>

#include "ObjDetector.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{


/*---------------------------------------------------------------------------*\
                      Class PARSeqDetector Declaration
\*---------------------------------------------------------------------------*/

class PARSeqDetector
:
	public ObjDetector
{
public:

	using Base = ObjDetector;

protected:

	// Protected data

		//- The max length of the recognized list of characters, i.e.
		//	the number of output positions.
		//
		//	For more info, see:
		//	https://github.com/baudm/parseq/tree/main?tab=readme-ov-file#method-tldr
		const int nPos_ {26};

	// Protected member functions

		//- Extract inference results
		virtual void extract() override;

		//- Store results.
		//	NOTE: no-op, everything stored during extraction
		virtual void store() override {};

public:

	// Public data

		//- The character set recognized by the model
		std::string charset
		{
			R"(0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!"#$%&'()*+,-./:;<=>?@[\]^_`{|}~)"
		};

	// Constructors

		//- Default constructor
		PARSeqDetector()
		{
			// no padding or cropping, the input image should be just the
			// word/character sequence which is to be evaluated/recognized
			resizeMode_ = Base::ResizeMode::ResizeRaw;
			// For more info, see:
			// https://github.com/baudm/parseq/blob/1902db043c029a7e03a3818c616c06600af574be/strhub/data/module.py#L69
			scale = Base::Vec3<> {0.5/255.0, 0.5/255.0, 0.5/255.0};
			mean = Base::Vec3<> {0.5*255.0, 0.5*255.0, 0.5*255.0};
		}

		//- Default copy constructor
		PARSeqDetector(const PARSeqDetector&) = default;

		//- Default move constructor
		PARSeqDetector(PARSeqDetector&&) = default;

	//- Destructor
	virtual ~PARSeqDetector() = default;

	// Member functions

	// Member operators

		//- Default copy assignment
		PARSeqDetector& operator=(const PARSeqDetector&) = default;

		//- Default move assignment
		PARSeqDetector& operator=(PARSeqDetector&&) = default;

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
