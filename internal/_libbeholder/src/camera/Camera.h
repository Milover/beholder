/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A wrapper class for a camera device.

SourceFiles
	Camera.cpp

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_CAMERA_H
#define BEHOLDER_CAMERA_H

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <memory>
#include <ratio>

#include <GenApi/Container.h>
#include <GenApi/INode.h>
#include <GenApi/INodeMap.h>
#include <pylon/Device.h>
#include <pylon/GrabResultPtr.h>
#include <pylon/InstantCamera.h>
#include <pylon/Parameter.h>

#include "Exception.h"
#include "ParamEntry.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{

enum class TriggerType
{
	Software,
	Unknown = -1
};

/*---------------------------------------------------------------------------*\
                          Class Camera Declaration
\*---------------------------------------------------------------------------*/

class Camera
{
private:

	// Private data

		//- Pylon instant camera
		Pylon::CInstantCamera cam_;
		//- Pylon grab result
		Pylon::CGrabResultPtr res_;

	// Private Member functions

protected:

	// Protected Member functions

		//- Get all parameters satisfying the condition 'c'.
		// The returned list of parameters is sorted by parameter name.
		template
		<
			typename Condition,
			typename... Nodemaps,
			typename = std::enable_if_t
			<
				(std::is_same_v<Nodemaps, GenApi::INodeMap> && ...)
			>,
			typename = std::enable_if_t
			<
				std::is_same_v<std::invoke_result_t<Condition, GenApi::INode*>, bool>
			>
		>
		void getParamsImpl
		(
			ParamList& params,
			Condition c,
			Nodemaps&... nodemaps
		) const;

		//- Execute a trigger.
		bool triggerImpl(TriggerType typ);

public:

	// Constructors

		//- Construct from a device.
		//	The device is attached and open after construction.
		Camera();

		//- Disable copy constructor
		Camera(const Camera&) = delete;

	//- Destructor
	~Camera() noexcept;

	// Member functions

		//- Acquire an image.
		//	WARNING: acquisition must be started manually, however,
		//	acquisition can be stopped automatically, eg. when a certain
		//	number of images has been acquired.
		template<typename Rep, typename Period = std::ratio<1>>
		bool acquire(const std::chrono::duration<Rep, Period>& timeout);

#ifndef NDEBUG
		//- Get reference to the underlying pylon camera
		Pylon::CInstantCamera& getRef() noexcept;
#endif

		//- Get reference to the acquired result
		const Pylon::CGrabResultPtr& getResult() noexcept
		{
			return res_;
		}

		//- Get camera parameters
		ParamList getParams(ParamAccessMode mode = ParamAccessMode::ReadWrite);

		//- Initialize camera device
		bool init(Pylon::IPylonDevice* d) noexcept;

		//- Return acquisition state
		bool isAcquiring() const noexcept;

		//- Check if the camera is initialized (device attached and open).
		bool isInitialized() const noexcept;

		//- Check if the camera device is attached.
		bool isAttached() const noexcept;

		//- Set camera parameters in the order provided.
		//	Returns true if no errors ocurred.
		bool setParams(const ParamList& params) noexcept;

		//- Start image acquisition
		bool startAcquisition() noexcept;

		//- Start image acquisition and stop after nImages have been acquired.
		bool startAcquisition(std::size_t nImages) noexcept;

		//- Stop image acquisition
		void stopAcquisition() noexcept;

		//- Executes a trigger.
		bool trigger(TriggerType typ = TriggerType::Software) noexcept;

		//- Waits for the trigger to become ready and then executes the trigger.
		template<typename Rep, typename Period = std::ratio<1>>
		bool waitAndTrigger
		(
			const std::chrono::duration<Rep, Period>& timeout = std::chrono::seconds {0},
			TriggerType typ = TriggerType::Software
		) noexcept;

	// Member operators

		//- Disable copy assignment
		Camera& operator=(const Camera&) = delete;

};

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

template<typename Rep, typename Period>
bool Camera::acquire(const std::chrono::duration<Rep, Period>& timeout)
{
	if (!isAttached())
	{
		throw Exception {"no camera device attached"};
	}
	if (!isAcquiring())
	{
		throw Exception {"acquisition not started"};
	}
	bool success
	{
		cam_.RetrieveResult
		(
			std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count(),
			res_,
			Pylon::TimeoutHandling_Return
		)
	};
	if (success && res_->GrabSucceeded())
	{
		if (res_->HasCRC() && !res_->CheckCRC())
		{
			std::cerr << "CRC check failed" << std::endl;
		}
		else
		{
			return true;
		}
	}
	else if (success)
	{
		std::cerr << "error code: " << res_->GetErrorCode() << '\t'
				  << res_->GetErrorDescription() << std::endl;
	}
	else
	{
		std::cerr << "acquisition timed out" << std::endl;
	}
	return false;
}

template<typename Rep, typename Period>
bool Camera::waitAndTrigger
(
	const std::chrono::duration<Rep, Period>& timeout,
	TriggerType typ
) noexcept
{
	try
	{
		if (!cam_.CanWaitForFrameTriggerReady())
		{
			std::cerr << "could not execute trigger: "
					  << "camera device cannot wait for trigger" << std::endl;
		}
		if
		(
			cam_.WaitForFrameTriggerReady
			(
				std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count(),
				Pylon::TimeoutHandling_Return
			)
		)
		{
			return triggerImpl(typ);
		}
	}
	catch(const Pylon::GenericException& e)
	{
		std::cerr << "could not execute trigger: " << e.what() << std::endl;
	}
	catch(const Exception& e)
	{
		std::cerr << "could not execute trigger: " << e.what() << std::endl;
	}
	catch(...)
	{
		std::cerr << "could not execute trigger: " << std::endl;
	}
	return false;
}

// * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * * //

template<typename Condition, typename... Nodemaps, typename, typename>
void Camera::getParamsImpl
(
	ParamList& params,
	Condition c,
	Nodemaps&... nodemaps
) const
{
	using NList = GenApi::NodeList_t;
	using NMap = GenApi::INodeMap;

	NList tmp;
	auto appendNodes = [&tmp](NList& nodes, const NMap& map) -> void
	{
		map.GetNodes(tmp);
		nodes.reserve(nodes.size() + tmp.size());
		for (auto&& n : tmp)
		{
			// doesn't have 'emplace_back()',
			// or 'value_type' so can't use std::back_inserter
			nodes.push_back(std::forward<GenApi::INode*>(n));
		}
	};
	NList nodes;
	(appendNodes(nodes, nodemaps), ...);

	params.reserve(nodes.size());	// guesstimate, we'll shrink it later
	for (const auto& n : nodes)
	{
		if (!c(n))
		{
			continue;
		}
		ParamType typ {convertInterfaceType(n->GetPrincipalInterfaceType())};
		if (typ == ParamType::Unknown)
		{
			continue;
		}
		Pylon::CParameter par {n};
		params.emplace_back
		(
			par.GetInfo(Pylon::ParameterInfo_Name).c_str(),
			par.ToString().c_str(),
			typ
		);
	}
	params.shrink_to_fit();

	std::sort
	(
		params.begin(),
		params.end(),
		[](const auto& a, const auto& b) -> bool { return a.name < b.name; }
	);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
