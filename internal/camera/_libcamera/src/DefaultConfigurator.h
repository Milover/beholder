/*---------------------------------------------------------------------------*\

	camera - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A wrapper class for image processing functions.

SourceFiles
	DefaultConfigurator.cpp

\*---------------------------------------------------------------------------*/

#ifndef CAMERA_DEFAULT_CONFIGURATOR_H
#define CAMERA_DEFAULT_CONFIGURATOR_H

#include <GenApi/INodeMap.h>
#include <pylon/ConfigurationEventHandler.h>
#include <pylon/InstantCamera.h>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace camera
{

/*---------------------------------------------------------------------------*\
                    Class DefaultConfigurator Declaration
\*---------------------------------------------------------------------------*/

class DefaultConfigurator
:
	public Pylon::CConfigurationEventHandler
{
protected:

	// Private data

	// Protected Member functions

		// Apply the configuration
		virtual void applyConfiguration(GenApi::INodeMap& nodemap) const;

public:

	//- Public data

	// Constructors

		//- Default constructor
		DefaultConfigurator() = default;

		//- Default copy constructor
		DefaultConfigurator(const DefaultConfigurator&) = default;

	//- Destructor
	virtual ~DefaultConfigurator() = default;

	// Member functions

		// Apply configuration right after the camera device is opened
		virtual void OnOpened(Pylon::CInstantCamera& cam) override;

	// Member operators

		//- Default copy assignment
		DefaultConfigurator& operator=(const DefaultConfigurator&) = default;

};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace camera

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
