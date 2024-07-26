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

bool Camera::triggerImpl(TriggerType typ)
{
	switch (typ)
	{
		case TriggerType::Software:
		{
			cam_.ExecuteSoftwareTrigger();
			return true;
			break;
		}
		case TriggerType::Unknown:
		{
			return false;
			break;
		}
	}
	return false;
}

// * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * * //

Camera::Camera()
{
	cam_.RegisterConfiguration
	(
		new camera::DefaultConfigurator,
		Pylon::RegistrationMode_ReplaceAll,
		Pylon::Cleanup_Delete
	);
}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Camera::~Camera() noexcept
{
	cam_.DestroyDevice();
}

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

#ifndef NDEBUG
Pylon::CInstantCamera& Camera::getRef() noexcept
{
	return cam_;
}
#endif

ParamList Camera::getParams(ParamAccessMode mode)
{
	ParamList params;
	bool (*condition)(GenApi::INode*) {nullptr};
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

bool Camera::init(Pylon::IPylonDevice* d) noexcept
{
	try
	{
		cam_.Attach(d, Pylon::Cleanup_Delete);
		cam_.Open();
		return true;
	}
	catch(const Pylon::GenericException& e)
	{
		std::cerr << "could not initialize camera: " << e.what() << std::endl;
	}
	catch(const Exception& e)
	{
		std::cerr << "could not initialize camera: " << e.what() << std::endl;
	}
	catch(...)
	{
		std::cerr << "could not initialize camera" << std::endl;
	}
	return false;
}

bool Camera::isInitialized() const noexcept
{
	return isAttached() && cam_.IsOpen();
}

bool Camera::isAttached() const noexcept
{
	return cam_.IsPylonDeviceAttached() && !cam_.IsCameraDeviceRemoved();
}

bool Camera::setParams(const ParamList& params) noexcept
{
	if (!isInitialized())
	{
		std::cerr << "could not set parameters, camera uninitialized" << std::endl;
		return false;
	}
	bool ok {true};
	for (const auto& p : params)
	{
		try
		{
			Pylon::CParameter par {cam_.GetNodeMap(), p.name.c_str()};
			par.FromString(p.value.c_str());
		}
		catch(const Pylon::GenericException& e)
		{
			ok = false;
			std::cerr << "could not set \"" << p.name << ": " << e.what() << std::endl;
		}
		catch(...)
		{
			ok = false;
			std::cerr << "could not set \"" << p.name << "\"" << std::endl;
		}
	}
	return ok;
}

bool Camera::startAcquisition() noexcept
{
	try
	{
		cam_.StartGrabbing();
		return true;
	}
	catch(const Pylon::GenericException& e)
	{
		std::cerr << "could not start acquisition: " << e.what() << std::endl;
	}
	catch(...)
	{
		std::cerr << "could not start acquisition" << std::endl;
	}
	return false;
}

bool Camera::startAcquisition(std::size_t nImages) noexcept
{
	try
	{
		cam_.StartGrabbing(nImages);
		return true;
	}
	catch(const Pylon::GenericException& e)
	{
		std::cerr << "could not start acquisition: " << e.what() << std::endl;
	}
	catch(...)
	{
		std::cerr << "could not start acquisition" << std::endl;
	}
	return false;
}

void Camera::stopAcquisition() noexcept
{
	cam_.StopGrabbing();
}

bool Camera::trigger(TriggerType typ) noexcept
{
	try
	{
		return triggerImpl(typ);
	}
	catch(const Pylon::GenericException& e)
	{
		std::cerr << "could not execute trigger: " << e.what() << std::endl;
	}
	catch(const Exception& e)
	{
		std::cerr << "could not execute trigger: " << e.what() << std::endl;
	}
	catch(...)
	{
		std::cerr << "could not execute trigger: " << std::endl;
	}
	return false;
}

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace camera

// ************************************************************************* //
