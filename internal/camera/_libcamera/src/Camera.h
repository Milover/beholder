/*---------------------------------------------------------------------------*\

	camera - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A wrapper class for a camera device.

SourceFiles
	Camera.cpp

\*---------------------------------------------------------------------------*/

#ifndef CAMERA_CAMERA_H
#define CAMERA_CAMERA_H

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <ratio>
#include <utility>

#include <GenApi/Container.h>
#include <GenApi/INode.h>
#include <GenApi/INodeMap.h>
#include <pylon/Device.h>
#include <pylon/GrabResultPtr.h>
#include <pylon/InstantCamera.h>
#include <pylon/Parameter.h>

#include "ParamEntry.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace camera
{

/*---------------------------------------------------------------------------*\
                          Class Camera Declaration
\*---------------------------------------------------------------------------*/

class Camera
{
private:

	// Private data

		//- Pylon instant camera handle
		Pylon::CInstantCamera* cam_;

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

public:

	// Constructors

		//- Construct from a device
		Camera(Pylon::IPylonDevice* d);

		//- Disable copy constructor
		Camera(const Camera&) = delete;

	//- Destructor
	~Camera();

	// Member functions

		//- Acquire an image
		template<typename Rep, typename Period = std::ratio<1>>
		bool acquire
		(
			Pylon::CGrabResultPtr& result,
			const std::chrono::duration<Rep, Period>& timeout
		) const;

		//- Get camera parameters
		ParamList getParams(ParamAccessMode mode = ParamAccessMode::ReadWrite) const;

		//- Return acquisition state
		bool isAcquiring() const noexcept;

		//- Check if the camera is valid (device attached and open).
		bool isValid() const noexcept;

		//- Set camera parameters in the order provided.
		//	Returns true if no errors ocurred.
		bool setParams(const ParamList& params) const noexcept;

		//- Start image acquisition
		void startAcquisition() const;

		//- Start image acquisition and stop after nImages have been acquired.
		void startAcquisition(std::size_t nImages) const;

		//- Stop image acquisition
		void stopAcquisition() const noexcept;

	// Member operators

		//- Disable copy assignment
		Camera& operator=(const Camera&) = delete;

};

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

template<typename Rep, typename Period>
bool Camera::acquire
(
	Pylon::CGrabResultPtr& result,
	const std::chrono::duration<Rep, Period>& timeout
) const
{
	return cam_->RetrieveResult
	(
		std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count(),
		result,
		Pylon::TimeoutHandling_Return
	);
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

} // End namespace camera

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
