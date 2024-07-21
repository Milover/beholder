/*---------------------------------------------------------------------------*\

	camera - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <stdexcept>

#include <GenApi/INodeMap.h>
#include <pylon/ConfigurationHelper.h>
#include <pylon/InstantCamera.h>
#include <pylon/ParameterIncludes.h>

#include "DefaultConfigurator.h"
#include "Exception.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace camera
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

	// set pixel format to 8-bit
	Pylon::CEnumParameter(nodemap, "PixelFormat").SetValue("BayerRG8");
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

} // End namespace camera

// ************************************************************************* //
