/*---------------------------------------------------------------------------*\

	camera - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A wrapper class for a camera image.

SourceFiles
	Image.cpp

\*---------------------------------------------------------------------------*/

#ifndef CAMERA_IMAGE_H
#define CAMERA_IMAGE_H

#include <cstdint>

#include <pylon/GrabResultPtr.h>
#include <pylon/PylonImage.h>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace camera
{

/*---------------------------------------------------------------------------*\
                          Class Image Declaration
\*---------------------------------------------------------------------------*/

class Image
{
private:

	// Private data

		//- Pylon image
		Pylon::CPylonImage img_;

	// Private Member functions

public:

	// Public data

		//- Pylon image ID
		const int64_t id;

	// Constructors

		//- Construct from a grab result
		Image(const Pylon::CGrabResultPtr& r);

		//- Disable copy constructor
		Image(const Image&) = delete;

	//- Destructor
	~Image() = default;

	// Member functions

#ifndef NDEBUG
		//- Get reference to the underlying image
		Pylon::CPylonImage& getRef() noexcept;
#endif

	// Member operators

		//- Disable copy assignment
		Image& operator=(const Image&) = delete;

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace camera

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
