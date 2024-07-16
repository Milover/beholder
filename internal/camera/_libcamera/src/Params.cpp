/*---------------------------------------------------------------------------*\

	camera - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <string>
#include <vector>

#include <pylon/PylonIncludes.h>

#include "ParamEntry.h"
#include "Params.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

namespace camera
{

// * * * * * * * * * * * * * * * * Functions * * * * * * * * * * * * * * * * //

ParamList getReadableParams(const GenApi::INodeMap& nodemap)
{
	return getParams
	(
		nodemap,
		[](GenApi::INode* n) -> bool
		{
			return GenApi::IsAvailable(n)
				&& n->IsFeature()
				&& GenApi::IsReadable(n);
		}
	);
}

ParamList getWritableParams(const GenApi::INodeMap& nodemap)
{
	return getParams
	(
		nodemap,
		[](GenApi::INode* n) -> bool
		{
			return GenApi::IsAvailable(n)
				&& n->IsFeature()
				&& GenApi::IsReadable(n)
				&& GenApi::IsWritable(n);
		}
	);
}

void setParams(GenApi::INodeMap& nodemap, const ParamList& params)
{
	// TODO: should probably lock here
	for (const auto& p : params)
	{
		py::CParameter par {nodemap, p.name.c_str()};
		par.FromString(p.value.c_str());	// throws
	}
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace camera

// ************************************************************************* //
