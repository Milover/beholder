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

#include <cstdlib>

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
		const std::size_t id;
		//- Number of image rows
		const std::size_t rows;
		//- Number of image columns
		const std::size_t cols;

	// Constructors

		//- Construct from a grab result
		Image(const Pylon::CGrabResultPtr& r);

		//- Disable copy constructor
		Image(const Image&) = delete;

	//- Destructor
	~Image() = default;

	// Member functions

		//- Get a pointer to the underlying bytes of the image
		unsigned char* getBuffer() noexcept;

		//- Check if the image is monochrome
		std::size_t getStep() const noexcept;

		//- Get image step size in bytes
		//	FIXME: provisional implementation
		bool isMonochrome() const noexcept;

		//- Write image to disc
		bool write(const std::string& filename) noexcept;

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
