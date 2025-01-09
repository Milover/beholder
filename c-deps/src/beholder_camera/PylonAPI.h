/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A wrapper class for the pylon runtime manager.

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_PYLON_H
#define BEHOLDER_PYLON_H

#include <pylon/PylonBase.h>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{

/*---------------------------------------------------------------------------*\
                    Class PylonAPI Declaration
\*---------------------------------------------------------------------------*/

class PylonAPI
{
private:

	// Private data

		//- Pylon API handle
		Pylon::PylonAutoInitTerm py_;

public:

	// Constructors

		//- Default constructor
		PylonAPI() = default;

		//- Disable copy constructor
		PylonAPI(const PylonAPI&) = delete;

	//- Destructor
	~PylonAPI() = default;

	// Member operators

		//- Disable copy assignment
		PylonAPI& operator=(const PylonAPI&) = delete;

};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
