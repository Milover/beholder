/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A simple class for holding various image detection results

SourceFile
	Result.h

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_RESULT_H
#define BEHOLDER_RESULT_H

#ifdef __cplusplus
#include <cstring>
#include <cstddef>
#include <string>
#else
#include <stddef.h>
#endif

#include "Rectangle.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef __cplusplus
namespace beholder
{
namespace capi
{
extern "C"
{
#endif

/*---------------------------------------------------------------------------*\
                       Class Result Declaration
\*---------------------------------------------------------------------------*/

// We use this guy only from Go because the char* is cancer.
typedef struct
{
	// TODO: should we hold an image or a reference to an image?

	//- Recognized/detected text
	char* text;

	//- Bounding boxes detected by algorithms and/or NNs
	Rectangle box;

	//- Confidence of the result
	double confidence;

}
Result;

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

//- Free memory held by r.
inline void Result_Delete(Result* r)
{
	if (r->text)
	{
		delete[] r->text;
		r->text = nullptr;
	}
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef __cplusplus
} // End namespace capi
} // End extern "C"
#endif

/*---------------------------------------------------------------------------*\
                          Class Result Declaration
\*---------------------------------------------------------------------------*/

// We always use this guy where we can.
//
// TODO: instead of this, we should inherit from capi::Result and have a
// wrapper class for the char* which acts like a std::string.
class Result
{
public:

	// Public data

		// TODO: should we hold an image or a reference to an image?

		//- Recognized/detected text
		std::string text;

		//- Bounding boxes detected by algorithms and/or NNs
		capi::Rectangle box;

		//- Confidence of the result
		double confidence;


	// Public member functions

		//- Return a capi::Result copy
		//
		//	WARNING: this function is intended for use ONLY from C-APIs,
		//	i.e. from Go.
		//	It is marked as deprecated so that the compiler can yell at us
		//	if we accidentally use it.
		[[nodiscard, deprecated("unsafe C-API function")]]
		capi::Result toC() const
		{
			char* ch {new char[text.size() + 1]};
			std::strcpy(ch, text.c_str());
			return capi::Result {ch, box, confidence};
		}
};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef __cplusplus
} // End namespace beholder
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
