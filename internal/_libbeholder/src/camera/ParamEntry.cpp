/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <algorithm>
#include <utility>

#include <GenApi/Types.h>

#include "ParamEntry.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

namespace beholder
{

// * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * * //

ParamEntry::ParamEntry
(
	const std::string& n,
	const std::string& v,
	const ParamType& t
)
:
	name{n},
	value{v},
	type{t}
{}

ParamEntry::ParamEntry(std::string&& n, std::string&& v, ParamType&& t)
:
	name{std::move(n)},
	value{std::move(v)},
	type{std::move(t)}
{}

ParamEntry::ParamEntry(const std::string& n, const std::string& v)
:
	name{n},
	value{v}
{}

ParamEntry::ParamEntry(std::string&& n, std::string&& v)
:
	name{std::move(n)},
	value{std::move(v)}
{}

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

ParamType convertInterfaceType(GenApi::EInterfaceType e)
{
	if (e == GenApi::intfIInteger)
	{
		return ParamType::Int;
	}
	if (e == GenApi::intfIBoolean)
	{
		return ParamType::Bool;
	}
	if (e == GenApi::intfICommand)
	{
		return ParamType::Command;
	}
	if (e == GenApi::intfIFloat)
	{
		return ParamType::Float;
	}
	if (e == GenApi::intfIString)
	{
		return ParamType::String;
	}
	if (e == GenApi::intfIEnumeration)
	{
		return ParamType::Enum;
	}
	if (e == GenApi::intfIRegister)
	{
		return ParamType::Register;
	}
	return ParamType::Unknown;
}

ParamEntry getParameter(const std::string& pName, const ParamList& list)
{
	auto found
	{
		std::find_if
		(
			list.begin(),
			list.end(),
			[&pName](const ParamEntry& p) -> bool { return p.name == pName; }
		)
	};
	if (found != list.end())
	{
		return *found;
	}
	return {};
}

bool hasParameter(const std::string& pName, const ParamList& list)
{
	return list.end() != std::find_if
	(
		list.begin(),
		list.end(),
		[&pName](const ParamEntry& p) -> bool { return p.name == pName; }
	);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// ************************************************************************* //
