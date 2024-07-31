/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <cassert>
#include <chrono>
#include <cstdlib>
#include <iostream>

#include <pylon/PylonIncludes.h>

#include "Camera.h"
#include "Exception.h"
#include "ImageProcessor.h"
#include "ParamEntry.h"
#include "PylonAPI.h"
#include "TransportLayer.h"

#include "Test.h"

// * * * * * * * * * * * * * * * * Constants * * * * * * * * * * * * * * * * //

namespace beholder
{

// acA2440-20gm
const std::string CameraSN {"24491241"};
const std::string CameraMAC {"0030534487E9"};

// acA4024-8gc
//const std::string CameraSN {"23096460"};
//const std::string CameraMAC {"0030532F3F8C"};

const std::string CameraSubnetMask {"255.255.255.0"};
const std::string CameraGateway {"0.0.0.0"};
const std::string CameraIP {"192.168.1.85"};	// the new IP

const std::size_t CameraNImages {10};

const beholder::ParamList CameraParameters
{
	{"AcquisitionMode",            "Continuous",                   beholder::ParamType::Enum},

	{"TriggerSelector",            "FrameStart",                   beholder::ParamType::Enum},
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

		// create the image processor
		beholder::ImageProcessor ip {};

		// Acquire image(s)
		//cam.startAcquisition(beholder::CameraNImages);
		cam.startAcquisition(1);
		while (cam.isAcquiring())
		{
			std::cout << "Acquiring...\n";
			if (!cam.acquire(std::chrono::seconds {2}))
			{
				continue;
			}
			std::cout << "image id:   " << cam.getResult()->GetID() << '\n'
					  << "image size: " << static_cast<double>(cam.getResult()->GetImageSize()) / 1000000.0 << "MB\n"
					  << '\n';

			if (!ip.receiveAcquisitionResult(cam.getResult()))
			{
				std::cerr << "failed to convert acquired image\n";
				continue;
			}
			// write
			if
			(
				!ip.writeImage(std::string{"img_"} + std::to_string(ip.getImageID()) + ".png")
			)
			{
				std::cerr << "failed to write image\n";
			}
			// write using the Pylon API
			if
			(
				!ip.writeAcquisitionResult
				(
					cam.getResult(),
					std::string{"py_img_"} + std::to_string(ip.getImageID()) + ".png"
				)
			)
			{
				std::cerr << "failed to write image with Pylon API\n";
			}
		}
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
