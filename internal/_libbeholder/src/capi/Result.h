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
#include <type_traits>

#include "Traits.h"
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
void Result_Delete(Result* r);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef __cplusplus
} // End namespace capi
} // End extern "C"

/*---------------------------------------------------------------------------*\
                          Class Result Declaration
\*---------------------------------------------------------------------------*/

// We always use this guy where we can.
//
// FIXME: we don't like how this is implemented.
class Result
{
public:

	using CAPI = capi::Result;

	// Public data

	//- Recognized/detected text
	std::string text;

	//- Bounding boxes detected by algorithms and/or NNs
	Rectangle box;

	//- Confidence of the result
	double confidence;

	// Constructors

		//- Default constructor
		Result() = default;

		//- Default copy constructor
		Result(const Result&) = default;

		//- Default move constructor
		Result(Result&&) = default;

		//- Construct by forwarding components
		template<typename Txt, typename Box, typename Conf>
		Result(Txt&& t, Box&& b, Conf&& c)
		:
			text {std::forward<Txt>(t)},
			box {std::forward<Box>(b)},
			confidence {std::forward<Conf>(c)}
		{}

		//- Construct by copying a capi::Result
		Result(const capi::Result& r)
		:
			text {r.text},
			box {r.box},
			confidence {r.confidence}
		{}

	//- Destructor
	~Result() = default;

	// Public member functions

		//- Return a capi::Result copy
		capi::Result toC() const
		{
			char* ch {new char[text.size() + 1]};
			std::strcpy(ch, text.c_str());
			return capi::Result {ch, box.cRef(), confidence};
		}

	// Member operators

		//- Default copy assignment operator
		Result& operator=(const Result&) = default;

		//- Default move assignment operator
		Result& operator=(Result&&) = default;

};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
