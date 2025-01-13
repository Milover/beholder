// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#include "beholder/camera/ParamEntry.h"

#include <GenApi/Types.h>

#include <algorithm>
#include <type_traits>
#include <utility>

#include "beholder/util/Enums.h"

namespace beholder {

namespace {
// Static checks which enforce compliance between our parameter types and
// the GenICam ones.
using Typ = ParamType;
static_assert(Typ::_val == GenApi::intfIValue);
static_assert(Typ::_base == GenApi::intfIBase);
static_assert(Typ::Int == GenApi::intfIInteger);
static_assert(Typ::Bool == GenApi::intfIBoolean);
static_assert(Typ::Cmd == GenApi::intfICommand);
static_assert(Typ::Float == GenApi::intfIFloat);
static_assert(Typ::Str == GenApi::intfIString);
static_assert(Typ::Reg == GenApi::intfIRegister);
static_assert(Typ::_cat == GenApi::intfICategory);
static_assert(Typ::Enum == GenApi::intfIEnumeration);
static_assert(Typ::_entry == GenApi::intfIEnumEntry);
static_assert(Typ::_port == GenApi::intfIPort);
}  // namespace

ParamEntry getParameter(const std::string& pName, const ParamList& list) {
	auto found{std::find_if(
		list.begin(), list.end(),
		[&pName](const ParamEntry& p) -> bool { return p.name == pName; })};
	if (found != list.end()) {
		return *found;
	}
	return {};
}

bool hasParameter(const std::string& pName, const ParamList& list) {
	return list.end() != std::find_if(list.begin(), list.end(),
									  [&pName](const ParamEntry& p) -> bool {
										  return p.name == pName;
									  });
}

}  // namespace beholder
