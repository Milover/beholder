/*---------------------------------------------------------------------------*\

	camera - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <iostream>

#include <GenApi/INode.h>
#include <pylon/Device.h>
#include <pylon/InstantCamera.h>
#include <pylon/ECleanup.h>
#include <pylon/ERegistrationMode.h>
#include <pylon/ETimeoutHandling.h>
#include <pylon/Parameter.h>

#include "Camera.h"
#include "DefaultConfigurator.h"
#include "Exception.h"
#include "ParamEntry.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace camera
{

// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

// * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * * //

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * * //

Camera::Camera(Pylon::IPylonDevice* d)
:
	cam_ {d, Pylon::Cleanup_Delete}
{
	cam_.RegisterConfiguration
	(
		new camera::DefaultConfigurator,
		Pylon::RegistrationMode_ReplaceAll,
		Pylon::Cleanup_Delete
	);
	cam_.Open();
}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Camera::~Camera()
{
	cam_.DestroyDevice();
}

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

ParamList Camera::getParams(ParamAccessMode mode)
{
	ParamList params;
	bool (*condition)(GenApi::INode*);
	switch (mode)
	{
		case ParamAccessMode::Read:
		{
			condition = +[](GenApi::INode* n) -> bool
			{
				return GenApi::IsAvailable(n)
					&& n->IsFeature()
					&& GenApi::IsReadable(n);
			};
			break;
		}
		case ParamAccessMode::ReadWrite:
		{
			condition = +[](GenApi::INode* n) -> bool
			{
				return GenApi::IsAvailable(n)
					&& n->IsFeature()
					&& GenApi::IsReadable(n)
					&& GenApi::IsWritable(n);
			};
			break;
		}
		case ParamAccessMode::Unknown:
		{
			condition = +[](GenApi::INode*) -> bool { return false; };
			break;
		}
	}
	getParamsImpl
	(
		params,
		condition,
		cam_.GetNodeMap(),
		cam_.GetTLNodeMap(),
		cam_.GetStreamGrabberNodeMap(),
		cam_.GetEventGrabberNodeMap(),
		cam_.GetInstantCameraNodeMap()
	);
	return params;
}

bool Camera::isAcquiring() const noexcept
{
	return cam_.IsGrabbing();
}

bool Camera::isValid() const noexcept
{
	return cam_.IsPylonDeviceAttached() && !cam_.IsCameraDeviceRemoved();
}

bool Camera::setParams(const ParamList& params) noexcept
{
	bool ok {true};
	for (const auto& p : params)
	{
		try
		{
			Pylon::CParameter par {cam_.GetNodeMap(), p.name.c_str()};
			par.FromString(p.value.c_str());
		}
		catch(...)
		{
			ok = false;
			std::cerr << "could not set \"" << p.name << "\"\n";
			// TODO: log error
		}
	}
	return ok;
}

void Camera::startAcquisition()
{
	cam_.StartGrabbing(Pylon::GrabStrategy_LatestImageOnly);
}

void Camera::startAcquisition(std::size_t nImages)
{
	cam_.StartGrabbing(nImages, Pylon::GrabStrategy_LatestImageOnly);
}

void Camera::stopAcquisition() noexcept
{
	cam_.StopGrabbing();
}

#ifndef NDEBUG
Pylon::CInstantCamera& Camera::getRef() noexcept
{
	return cam_;
}
#endif

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace camera

// ************************************************************************* //
