/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <cstdlib>

#include <pylon/GrabResultPtr.h>
#include <pylon/PylonImage.h>

#include "Image.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
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

std::size_t Image::getStep() const noexcept
{
	std::size_t step;
	if (img_.GetStride(step))
	{
		return step;
	}
	return 0ul;
}

bool Image::isMonochrome() const noexcept
{
	switch (img_.GetPixelType())
	{
		case Pylon::PixelType_Mono1packed:
		case Pylon::PixelType_Mono2packed:
		case Pylon::PixelType_Mono4packed:
		case Pylon::PixelType_Mono8:
		case Pylon::PixelType_Mono8signed:
		case Pylon::PixelType_Mono10:
		case Pylon::PixelType_Mono10packed:
		case Pylon::PixelType_Mono10p:
		case Pylon::PixelType_Mono12:
		case Pylon::PixelType_Mono12packed:
		case Pylon::PixelType_Mono12p:
		case Pylon::PixelType_Mono16:
			return true;
			break;
		default:
			return false;
			break;
	}
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

} // End namespace beholder

// ************************************************************************* //
