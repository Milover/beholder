/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A wrapper class for image processing functions.

SourceFiles
	TransportLayer.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_TRANSPORT_LAYER_H
#define BEHOLDER_TRANSPORT_LAYER_H

#include <string>

#include <pylon/Device.h>
#include <pylon/TransportLayer.h>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{

//- Transport layer types (device classes)
enum class DeviceClass : int
{
	GigE,
	Emulated,
	//USB,
	Unknown = -1
};

//- Types of device designators
enum class DeviceDesignator : int
{
	MAC,			// device MAC address
	SN,				// device serial number
	//IP,			// device IP address
	Unknown = -1
};

/*---------------------------------------------------------------------------*\
                    Class TransportLayer Declaration
\*---------------------------------------------------------------------------*/

class TransportLayer
{
private:

	// Private data

		//- Pylon transport layer handle
		Pylon::ITransportLayer* tl_ {nullptr};

	// Private Member functions

protected:

	// Protected member functions

		//- Find and create a device with the provided designator.
		Pylon::IPylonDevice* createDeviceImpl
		(
			const char* designator,
			DeviceDesignator ddt = DeviceDesignator::SN
		) const noexcept;

public:

	//- Public data

	// Constructors

		//- Construct from a device class
		TransportLayer() = default;

		//- Disable copy constructor
		TransportLayer(const TransportLayer&) = delete;

	//- Destructor
	~TransportLayer();

	// Member functions

		//- Initialize camera device
		bool init(DeviceClass dc = DeviceClass::GigE) noexcept;

		//- Find and create a device with the provided designator.
		//	NOTE: device reset is attempted during creation.
		Pylon::IPylonDevice* createDevice
		(
			const std::string& designator,
			DeviceDesignator ddt = DeviceDesignator::SN,
			bool reboot = true
		) const noexcept;

		//- Find and create a device with the provided designator.
		//	NOTE: device reset is attempted during creation.
		Pylon::IPylonDevice* createDevice
		(
			const char* designator,
			DeviceDesignator ddt = DeviceDesignator::SN,
			bool reboot = true
		) const noexcept;

		//- Get the serial number of the first device found
		std::string getFirstSN() const noexcept;

#ifndef NDEBUG
		//- Return the underlying pointer
		Pylon::ITransportLayer* getPtr() const noexcept
		{
			return tl_;
		}
#endif


	// Member operators

		//- Disable copy assignment
		TransportLayer& operator=(const TransportLayer&) = delete;

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

//- Return a formatted string of the device designator
std::string formatDeviceDesignator(DeviceDesignator ddt);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
