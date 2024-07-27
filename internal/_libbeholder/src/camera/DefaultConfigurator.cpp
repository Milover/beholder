/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <iostream>
#include <stdexcept>

#include <GenApi/INodeMap.h>
#include <pylon/ConfigurationHelper.h>
#include <pylon/InstantCamera.h>
#include <pylon/ParameterIncludes.h>

#include "DefaultConfigurator.h"
#include "Exception.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{

// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

// * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * * //

void DefaultConfigurator::applyConfiguration(GenApi::INodeMap& nodemap) const
{
	// load default set
	Pylon::CEnumParameter(nodemap, "UserSetSelector").SetValue("Default");
	Pylon::CCommandParameter(nodemap, "UserSetLoad").Execute();

	// disable all triggers, image compression and streaming
	Pylon::CConfigurationHelper::DisableAllTriggers(nodemap);
	Pylon::CConfigurationHelper::DisableCompression(nodemap);
	Pylon::CConfigurationHelper::DisableGenDC(nodemap);

	// set pixel format to 8-bit single-channel
	// FIXME: whatever is set here needs to be propagated to Go/OpenCV
	// FIXME: wat do if neither can be set?
	Pylon::CEnumParameter pixelFormat {nodemap, "PixelFormat"};
	if (!pixelFormat.TrySetValue("BayerRG8"))
	{
		std::cerr << "could not set pixel format to: BayerRG8" << std::endl;
	}
	else if (!pixelFormat.TrySetValue("Mono8"))
	{
		std::cerr << "could not set pixel format to: Mono8" << std::endl;
	}
}

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

void DefaultConfigurator::OnOpened(Pylon::CInstantCamera& cam)
{
	try
	{
		applyConfiguration(cam.GetNodeMap());
		// Probe max packet size
		Pylon::CConfigurationHelper::ProbePacketSize(cam.GetStreamGrabberNodeMap());
	}
	catch(const Pylon::GenericException& e)
	{
		throw Exception {"could not apply 'Default' configuration: ", e};
	}
	catch(const std::exception& e)
	{
		throw Exception {"could not apply 'Default' configuration: ", e};
	}
	catch(...)
	{
		throw Exception {"could not apply 'Default' configuration"};
	}
}

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// ************************************************************************* //
