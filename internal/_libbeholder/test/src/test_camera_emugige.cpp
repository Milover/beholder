/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <cassert>
#include <chrono>
#include <iostream>
#include <stdlib.h>

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

const std::string EmuSN  {"0815-0000"};
const std::string GigESN {"24491241"};

const std::size_t CameraNImages {1};

} // end namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	setenv("PYLON_CAMEMU", "3", true);

	// Before using any pylon methods, the pylon runtime must be initialized.
	beholder::PylonAPI py {};

	try
	{
		// Create transport layers
		beholder::TransportLayer tlEmu {};
		if (!tlEmu.init(beholder::DeviceClass::Emulated))
		{
			std::cerr << "error: emulated transport layer initialization failed\n";
			return 1;
		}
		beholder::TransportLayer tlGigE {};
		if (!tlGigE.init(beholder::DeviceClass::GigE))
		{
			std::cerr << "error: gige transport layer initialization failed\n";
			return 1;
		}

		// Create camera devices
		beholder::Camera camEmu {};
		if
		(
			!camEmu.init
			(
				tlEmu.createDevice(beholder::EmuSN, beholder::DeviceDesignator::SN)
			)
		)
		{
			std::cerr << "error: emulated camera initialization failed\n";
			return 1;
		}
		beholder::Camera camGigE {};
		if
		(
			!camGigE.init
			(
				tlGigE.createDevice(beholder::GigESN, beholder::DeviceDesignator::SN)
			)
		)
		{
			std::cerr << "error: gige camera initialization failed\n";
			return 1;
		}

		assert(camEmu.isInitialized());
		assert(camGigE.isInitialized());
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
