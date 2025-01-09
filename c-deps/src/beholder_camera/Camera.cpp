/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <optional>

#include <GenApi/INode.h>
#include <pylon/Device.h>
#include <pylon/InstantCamera.h>
#include <pylon/ECleanup.h>
#include <pylon/ERegistrationMode.h>
#include <pylon/ETimeoutHandling.h>
#include <pylon/Parameter.h>
#include <pylon/PixelType.h>

#include "capi/RawImage.h"
#include "Camera.h"
#include "DefaultConfigurator.h"
#include "Exception.h"
#include "ParamEntry.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
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
		new DefaultConfigurator,
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

bool Camera::cmdExecute(const std::string& cmd) noexcept
{
	return cmdExecute(cmd.c_str());
}

bool Camera::cmdExecute(const char* cmd) noexcept
{
	try
	{
		Pylon::CCommandParameter(cam_.GetNodeMap(), cmd).Execute();
		return true;
	}
	catch(const Pylon::GenericException& e)
	{
		std::cerr << "could not execute command: " << e.what() << std::endl;
	}
	catch(...)
	{
		std::cerr << "could not execute command" << std::endl;
	}
	return false;
}

bool Camera::cmdIsDone(const std::string& cmd) noexcept
{
	return cmdIsDone(cmd.c_str());
}

bool Camera::cmdIsDone(const char* cmd) noexcept
{
	try
	{
		return Pylon::CCommandParameter(cam_.GetNodeMap(), cmd).IsDone();
	}
	catch(const Pylon::GenericException& e)
	{
		std::cerr << "could not check command execution status: "
				  << e.what() << std::endl;
	}
	catch(...)
	{
		std::cerr << "could not check command execution status: " << std::endl;
	}
	return false;
}

std::optional<RawImage> Camera::getRawImage() noexcept
{
	// not sure if this can throw, so we're being careful
	try
	{
		if (!res_.IsValid())
		{
			return std::nullopt;
		}
		std::size_t step;
		return std::optional
		{
			RawImage
			{
				static_cast<std::size_t>(res_->GetID()),
				static_cast<int>(res_->GetHeight()),
				static_cast<int>(res_->GetWidth()),
				static_cast<std::int64_t>(res_->GetPixelType()),
				res_->GetBuffer(),
				res_->GetStride(step) ? step : 0ul,
				static_cast<std::size_t>(Pylon::BitPerPixel(res_->GetPixelType()))
			}
		};
	}
	catch(const Pylon::GenericException& e)
	{
		std::cerr << "could get raw image data: " << e.what() << std::endl;
	}
	catch(...)
	{
		std::cerr << "could get raw image data" << std::endl;
	}
	return std::nullopt;
}

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

bool Camera::startAcquisition(std::size_t nImages) noexcept
{
	// XXX: not sure what happens here if the camera gets disconnected
	if (isAcquiring())
	{
		return true;
	}
	try
	{
		if (nImages == 0)
		{
			cam_.StartGrabbing();
		}
		else
		{
			cam_.StartGrabbing(nImages);
		}
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

} // End namespace beholder

// ************************************************************************* //
