/*---------------------------------------------------------------------------*\

	camera - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	Common pylon parameter functions.

SourceFiles
	Params.cpp

\*---------------------------------------------------------------------------*/

#ifndef CAMERA_PARAMS_H
#define CAMERA_PARAMS_H

#include <algorithm>
#include <string>
#include <type_traits>
#include <vector>

#include <pylon/PylonIncludes.h>

#include "ParamEntry.h"

namespace py = Pylon;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace camera
{

// * * * * * * * * * * * * * * * * Functions * * * * * * * * * * * * * * * * //

// Get all parameters satisfying the condition 'c'.
// The returned list of parameters is sorted by parameter name.
template
<
	typename Condition,
	typename = std::enable_if_t
	<
		std::is_same_v<std::invoke_result_t<Condition, GenApi::INode*>, bool>
	>
>
ParamList getParams(const GenApi::INodeMap& nodemap, Condition c)
{
	// TODO: should probably lock here
	GenApi::NodeList_t nodes;
	nodemap.GetNodes(nodes);

	ParamList list;
	list.reserve(nodes.size());	// guesstimate, we'll shrink it later
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
		py::CParameter par (n);
		list.emplace_back
		(
			par.GetInfo(py::ParameterInfo_Name).c_str(),
			par.ToString().c_str(),
			typ
		);
		//par.Release();	// XXX: not sure if we can omit
	}
	list.shrink_to_fit();

	std::sort
	(
		list.begin(),
		list.end(),
		[](const ParamEntry& a, const ParamEntry& b) -> bool
		{
			return a.name < b.name;
		}
	);
	return list;
}

// Get all (available and implemented) readable device parameters.
ParamList getReadableParams(const GenApi::INodeMap& nodemap);

// Get all (available, implemented and readable) writable device parameters.
// FIXME: differences between this and py::CFeaturePersistence::Save(...).
ParamList getWritableParams(const GenApi::INodeMap& nodemap);

// Set all settable parameters.
// WARNING: no error checking, assumes all provided params
// are valid and settable; yolo
void setParams(GenApi::INodeMap& nodemap, const ParamList& params);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace camera

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
