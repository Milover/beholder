/*---------------------------------------------------------------------------*\

	camera - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <cassert>
#include <chrono>
#include <iostream>

#include <pylon/PylonIncludes.h>

#include "Camera.h"
#include "Exception.h"
#include "ParamEntry.h"
#include "PylonAPI.h"
#include "TransportLayer.h"

#include "Test.h"

// * * * * * * * * * * * * * * * * Constants * * * * * * * * * * * * * * * * //

namespace camera
{

// acA2440-20gm
//const std::string CameraSN {"24491241"};
//const std::string CameraMAC {"0030534487E9"};

// acA4024-8gc
const std::string CameraSN {"23096460"};
const std::string CameraMAC {"0030532F3F8C"};

const camera::ParamList CameraParameters
{
	{"AcquisitionMode",            "Continuous",                   camera::ParamType::Enum},

	{"TriggerMode",                "On",                           camera::ParamType::Enum},
	{"TriggerSource",              "Line1",                        camera::ParamType::Enum},
	{"TriggerActivation",          "RisingEdge",                   camera::ParamType::Enum},

	{"LineSelector",               "Line2",                        camera::ParamType::Enum},
	{"LineMode",                   "Output",                       camera::ParamType::Enum},
	{"LineSource",                 "FrameTriggerWait",             camera::ParamType::Enum},
	{"LineInverter",               "true",                         camera::ParamType::Bool},

	{"ExposureMode",               "Timed",                        camera::ParamType::Enum},
	{"ExposureTimeAbs",            "20000",                        camera::ParamType::Int},

	{"ChunkModeActive",            "true",                         camera::ParamType::Bool},
	{"ChunkSelector",              "PayloadCRC16",                 camera::ParamType::Enum},
	{"ChunkEnable",                "true",                         camera::ParamType::Bool}
};

} // end namespace camera

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	// Before using any pylon methods, the pylon runtime must be initialized.
	camera::PylonAPI py {};

	try
	{
		// Create transport layer
		camera::TransportLayer tl {};
		if (!tl.init(camera::DeviceClass::GigE))
		{
			std::cerr << "error: transport layer initialization failed\n";
			return 1;
		}
		// Create camera device and apply configuration
		camera::Camera cam {};
		if
		(
			!cam.init
			(
				tl.createDevice(camera::CameraMAC, camera::DeviceDesignator::MAC)
			)
		)
		{
			std::cerr << "error: camera initialization failed\n";
			return 1;
		}
		if (!cam.setParams(camera::CameraParameters))
		{
			std::cerr << "warning: encountered issues while setting parameters\n";
		}
		//camera::dumpParams(cam.getParams(camera::ParamAccessMode::Read));

		assert(cam.isInitialized());

		camera::writeParams
		(
			"params_readable",
			cam.getParams(camera::ParamAccessMode::Read)
		);
		Pylon::CFeaturePersistence::Save
		(
			"params_pylon",
			&cam.getRef().GetNodeMap()
		);
	}
	catch(const Pylon::GenericException& e)
	{
		std::cerr << "error: " << e.what() << '\n';
		return 1;
	}
	catch(const camera::Exception& e)
	{
		std::cerr << "error: " << e.what() << '\n';
		return 1;
	}
	catch(...)
	{
		std::cerr << "error: unknown\n";
		return 1;
	}

	return 0;
}

// ************************************************************************* //
