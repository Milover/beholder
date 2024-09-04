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

	// set preferred pixel format (8-bit single-channel)
	const char* formats[] {"BayerRG8", "Mono8"};
	Pylon::CEnumParameter pixelFormat {nodemap, "PixelFormat"};
	if (!pixelFormat.TrySetValue(formats))
	{
		std::cerr << "could not set default pixel format" << std::endl;
	}

	// reset image ROI
	if
	(
		!Pylon::CIntegerParameter {nodemap, "Width"}.TrySetToMaximum()
	 || !Pylon::CIntegerParameter {nodemap, "Height"}.TrySetToMaximum()
	 || !Pylon::CIntegerParameter {nodemap, "OffsetX"}.TrySetToMinimum()
	 || !Pylon::CIntegerParameter {nodemap, "OffsetY"}.TrySetToMinimum()
	)
	{
		std::cerr << "could not reset image ROI" << std::endl;
	}

	// see comments on Camera::isAcquiring() for more info
	const char* modes[] {"Continuous"};
	Pylon::CEnumParameter acquisitionMode {nodemap, "AcquisitionMode"};
	if (!acquisitionMode.TrySetValue(modes))
	{
		std::cerr << "could not set continuous acquisition mode" << std::endl;
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
