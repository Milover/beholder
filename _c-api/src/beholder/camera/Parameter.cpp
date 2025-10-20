// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include "beholder/camera/Parameter.h"

//#include <GenApi/Types.h>  // NOLINT

#include <algorithm>
#include <string>

//#include "beholder/util/Enums.h"

namespace beholder {
namespace camera {

// FIXME: disabled for now, until we figure out how to avoid including GenApi
// headers
namespace {
// Static checks which enforce compliance between our parameter types and
// the GenICam ones.
//using Typ = Type;
//static_assert(Typ::_val == GenApi::intfIValue);
//static_assert(Typ::_base == GenApi::intfIBase);
//static_assert(Typ::Int == GenApi::intfIInteger);
//static_assert(Typ::Bool == GenApi::intfIBoolean);
//static_assert(Typ::Cmd == GenApi::intfICommand);
//static_assert(Typ::Float == GenApi::intfIFloat);
//static_assert(Typ::Str == GenApi::intfIString);
//static_assert(Typ::Reg == GenApi::intfIRegister);
//static_assert(Typ::_cat == GenApi::intfICategory);
//static_assert(Typ::Enum == GenApi::intfIEnumeration);
//static_assert(Typ::_entry == GenApi::intfIEnumEntry);
//static_assert(Typ::_port == GenApi::intfIPort);
}  // namespace

Parameter getParameter(const std::string& pName, CParamSpan params) {
	auto found{std::find_if(
		params.begin(), params.end(),
		[&pName](const Parameter& p) -> bool { return p.name == pName; })};
	if (found == params.end()) {
		return {};
	}
	return *found;
}

bool hasParameter(const std::string& pName, CParamSpan params) {
	return params.end() != std::find_if(params.begin(), params.end(),
										[&pName](const Parameter& p) -> bool {
											return p.name == pName;
										});
}

}  // namespace camera
}  // namespace beholder
