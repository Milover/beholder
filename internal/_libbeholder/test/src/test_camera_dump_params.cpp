/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

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

namespace beholder
{

// acA2440-20gm
//const std::string CameraSN {"24491241"};
//const std::string CameraMAC {"0030534487E9"};

// acA4024-8gc
const std::string CameraSN {"23096460"};
const std::string CameraMAC {"0030532F3F8C"};

const beholder::ParamList CameraParameters
{
	{"AcquisitionMode",            "Continuous",                   beholder::ParamType::Enum},

	{"TriggerMode",                "On",                           beholder::ParamType::Enum},
	{"TriggerSource",              "Line1",                        beholder::ParamType::Enum},
	{"TriggerActivation",          "RisingEdge",                   beholder::ParamType::Enum},

	{"LineSelector",               "Line2",                        beholder::ParamType::Enum},
	{"LineMode",                   "Output",                       beholder::ParamType::Enum},
	{"LineSource",                 "FrameTriggerWait",             beholder::ParamType::Enum},
	{"LineInverter",               "true",                         beholder::ParamType::Bool},

	{"ExposureMode",               "Timed",                        beholder::ParamType::Enum},
	{"ExposureTimeAbs",            "20000",                        beholder::ParamType::Int},

	{"ChunkModeActive",            "true",                         beholder::ParamType::Bool},
	{"ChunkSelector",              "PayloadCRC16",                 beholder::ParamType::Enum},
	{"ChunkEnable",                "true",                         beholder::ParamType::Bool}
};

} // end namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	// Before using any pylon methods, the pylon runtime must be initialized.
	beholder::PylonAPI py {};

	try
	{
		// Create transport layer
		beholder::TransportLayer tl {};
		if (!tl.init(beholder::DeviceClass::GigE))
		{
			std::cerr << "error: transport layer initialization failed\n";
			return 1;
		}
		// Create camera device and apply configuration
		beholder::Camera cam {};
		if
		(
			!cam.init
			(
				tl.createDevice(beholder::CameraMAC, beholder::DeviceDesignator::MAC)
			)
		)
		{
			std::cerr << "error: camera initialization failed\n";
			return 1;
		}
		if (!cam.setParams(beholder::CameraParameters))
		{
			std::cerr << "warning: encountered issues while setting parameters\n";
		}
		//beholder::dumpParams(cam.getParams(beholder::ParamAccessMode::Read));

		assert(cam.isInitialized());

		beholder::writeParams
		(
			"params_readable",
			cam.getParams(beholder::ParamAccessMode::Read)
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
	catch(const beholder::Exception& e)
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
