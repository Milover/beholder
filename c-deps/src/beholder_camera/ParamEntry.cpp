// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#include "ParamEntry.h"

#include <GenApi/Types.h>

#include <algorithm>
#include <type_traits>
#include <utility>

namespace beholder {

namespace detail {
namespace ga = GenApi;
using ITyp = std::underlying_type_t<ga::EInterfaceType>;

// Static checks which enforce compliance between our parameter types and
// the GenICam ones.
static_assert(static_cast<ITyp>(ga::intfIValue) ==
			  static_cast<ITyp>(ParamType::_val));
static_assert(static_cast<ITyp>(ga::intfIBase) ==
			  static_cast<ITyp>(ParamType::_base));
static_assert(static_cast<ITyp>(ga::intfIInteger) ==
			  static_cast<ITyp>(ParamType::Int));
static_assert(static_cast<ITyp>(ga::intfIBoolean) ==
			  static_cast<ITyp>(ParamType::Bool));
static_assert(static_cast<ITyp>(ga::intfICommand) ==
			  static_cast<ITyp>(ParamType::Cmd));
static_assert(static_cast<ITyp>(ga::intfIFloat) ==
			  static_cast<ITyp>(ParamType::Float));
static_assert(static_cast<ITyp>(ga::intfIString) ==
			  static_cast<ITyp>(ParamType::Str));
static_assert(static_cast<ITyp>(ga::intfIRegister) ==
			  static_cast<ITyp>(ParamType::Reg));
static_assert(static_cast<ITyp>(ga::intfICategory) ==
			  static_cast<ITyp>(ParamType::_cat));
static_assert(static_cast<ITyp>(ga::intfIEnumeration) ==
			  static_cast<ITyp>(ParamType::Enum));
static_assert(static_cast<ITyp>(ga::intfIEnumEntry) ==
			  static_cast<ITyp>(ParamType::_entry));
static_assert(static_cast<ITyp>(ga::intfIPort) ==
			  static_cast<ITyp>(ParamType::_port));
}  // namespace detail

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
ParamEntry::ParamEntry(const std::string& n, const std::string& v,
					   const ParamType& t)
	: name{n}, value{v}, type{t} {}

ParamEntry::ParamEntry(std::string&& n, std::string&& v, ParamType&& t)
	: name{std::move(n)}, value{std::move(v)}, type{t} {}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
ParamEntry::ParamEntry(const std::string& n, const std::string& v)
	: name{n}, value{v} {}

ParamEntry::ParamEntry(std::string&& n, std::string&& v)
	: name{std::move(n)}, value{std::move(v)} {}

ParamType fromGenAPI(unsigned int i) {
	static_assert(std::is_same_v<detail::ITyp, decltype(i)>);

	const GenApi::EInterfaceType e{static_cast<GenApi::EInterfaceType>(i)};
	switch (e) {
		//case GenApi::intfIValue:
		//	return ParamType::Val;
		//case GenApi::intfIBase:
		//	return ParamType::Base;
		case GenApi::intfIInteger:
			return ParamType::Int;
		case GenApi::intfIBoolean:
			return ParamType::Bool;
		case GenApi::intfICommand:
			return ParamType::Cmd;
		case GenApi::intfIFloat:
			return ParamType::Float;
		case GenApi::intfIString:
			return ParamType::Str;
		case GenApi::intfIRegister:
			return ParamType::Reg;
		//case GenApi::intfICategory:
		//	return ParamType::Cat;
		case GenApi::intfIEnumeration:
			return ParamType::Enum;
		//case GenApi::intfIEnumEntry:
		//	return ParamType::Entry;
		//case GenApi::intfIPort:
		//	return ParamType::Port;
		default:
			return ParamType::Unknown;
	}
}

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
