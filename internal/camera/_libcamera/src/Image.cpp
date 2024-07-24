/*---------------------------------------------------------------------------*\

	camera - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <cstdlib>

#include <pylon/GrabResultPtr.h>
#include <pylon/PylonImage.h>

#include "Image.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace camera
{

// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

// * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * * //

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * * //

Image::Image(const Pylon::CGrabResultPtr& r)
:
	id {static_cast<std::size_t>(r->GetID())},
	rows {r->GetHeight()},
	cols {r->GetWidth()}
{
	img_.AttachGrabResultBuffer(r);
}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

unsigned char* Image::getBuffer() noexcept
{
	return static_cast<unsigned char*>(img_.GetBuffer());
}

std::size_t Image::getStep() const
{
	std::size_t step;
	if (img_.GetStride(step))
	{
		return step;
	}
	return 0ul;
}

bool Image::write(const std::string& filename) noexcept
{
	try
	{
		img_.Save(Pylon::ImageFileFormat_Png, filename.c_str());
		return true;
	}
	catch(...) { }
	return false;
}

#ifndef NDEBUG
Pylon::CPylonImage& Image::getRef() noexcept
{
	return img_;
}
#endif

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace camera

// ************************************************************************* //
