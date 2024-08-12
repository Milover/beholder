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
#include "ParamEntry.h"
#include "Processor.h"
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

const std::size_t CameraNImages {3};

const beholder::ParamList CameraParameters
{
	{"AcquisitionMode",            "Continuous",                   beholder::ParamType::Enum},

	{"TriggerSelector",            "FrameStart",                   beholder::ParamType::Enum},
	{"TriggerMode",                "On",                           beholder::ParamType::Enum},
	{"TriggerSource",              "Software",                     beholder::ParamType::Enum},

	{"ExposureMode",               "Timed",                        beholder::ParamType::Enum},
	{"ExposureTimeAbs",            "1500",                         beholder::ParamType::Int},

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
		//beholder::dumpParams(cam.getParams());
		//beholder::dumpParams(cam.getParams(beholder::ParamAccessMode::Read));

		assert(cam.isInitialized());

		// create the image processor
		beholder::Processor ip {};

		// Acquire image(s)
		cam.startAcquisition(beholder::CameraNImages);

		for (auto i {0ul}; i < beholder::CameraNImages; ++i)
		{
			std::cout << "Triggering...";
			auto start {std::chrono::system_clock::now()};
			if (!cam.waitAndTrigger(std::chrono::milliseconds {500}))
			{
				continue;
			}
			std::cout << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(std::chrono::system_clock::now() - start).count()
					  << "ms\n";

			std::cout << "Acquiring...\n";
			if (!cam.acquire(std::chrono::seconds {2}))
			{
				continue;
			}
			std::cout << "image id:   " << cam.getResult()->GetID() << '\n'
					  << "image size: " << static_cast<double>(cam.getResult()->GetImageSize()) / 1000000.0 << "MB\n";

			if (auto raw {cam.getRawImage()}; !raw || !ip.receiveRawImage(raw.value()))
			{
				std::cerr << "failed to convert acquired image\n";
				continue;
			}
			// write
			std::cout << "Writing...\n";
			if
			(
				!ip.writeImage(std::string{"img_"} + std::to_string(ip.getImageID()) + ".png")
			)
			{
				std::cerr << "failed to write PNG image\n";
			}
			if
			(
				!ip.writeImage(std::string{"img_"} + std::to_string(ip.getImageID()) + ".jpeg")
			)
			{
				std::cerr << "failed to write JPEG image\n";
			}
			// write using the Pylon API
//			if
//			(
//				!ip.writeAcquisitionResult
//				(
//					cam.getResult(),
//					std::string{"py_img_"} + std::to_string(ip.getImageID()) + ".png"
//				)
//			)
//			{
//				std::cerr << "failed to write image with Pylon API\n";
//			}

			std::cout << "Waiting for 2s\n";
			beholder::wait(std::chrono::seconds {2});

			std::cout << std::endl;
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
