// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

// Wrapper classes for GenICam parameters.

#ifndef BEHOLDER_CAMERA_PARAM_ENTRY_H
#define BEHOLDER_CAMERA_PARAM_ENTRY_H

#include <string>
#include <utility>
#include <vector>

#include "beholder/BeholderExport.h"

namespace beholder {

class ParamEntry;							// forward declaration
using ParamList = std::vector<ParamEntry>;	// handy-dandy typedef

// Supported GenICam parameter types.
enum class BH_API ParamType {
	_val,		  // GenAPI value (unused)
	_base,		  // GenAPI base (unused)
	Int,		  // GenAPI integer
	Bool,		  // GenAPI boolean
	Cmd,		  // GenAPI command
	Float,		  // GenAPI float
	Str,		  // GenAPI string
	Reg,		  // GenAPI register
	_cat,		  // GenAPI category (unused)
	Enum,		  // GenAPI enumeration
	_entry,		  // GenAPI enumeration entry (unused)
	_port,		  // GenAPI port (unused)
	Unknown = -1  // we don't actually know if this is ok; yolo
};

// Supported parameter access modes.
enum class BH_API ParamAccessMode { Read, ReadWrite, Unknown = -1 };

// ParamEntry represents a GenICam parameter.
class BH_API ParamEntry {
public:
	std::string name;					 // parameter name
	std::string value;					 // parameter value
	ParamType type{ParamType::Unknown};	 // parameter type

	// Construct from parts.
	template<typename Name, typename Value, typename Type = ParamType,
			 std::enable_if_t<std::is_constructible_v<std::string, Name> &&
								  std::is_constructible_v<std::string, Value> &&
								  std::is_constructible_v<ParamType, Type>,
							  bool> = true>
	ParamEntry(Name&& n, Value&& v, Type&& t = Type{ParamType::Unknown})
		: name{std::forward<Name>(n)},
		  value{std::forward<Value>(v)},
		  type{std::forward<Type>(t)} {};

	ParamEntry() = default;
	ParamEntry(const ParamEntry&) = default;
	ParamEntry(ParamEntry&&) = default;

	~ParamEntry() = default;

	ParamEntry& operator=(const ParamEntry&) = default;
	ParamEntry& operator=(ParamEntry&&) = default;
};

// Get a parameter named 'pName' from 'list'.
// Returns an empty ParamEntry if the parameter is not available.
[[nodiscard]] BH_API ParamEntry getParameter(const std::string& pName,
											 const ParamList& list);

// Check if a parameter named 'pName' is present in 'list'.
BH_API bool hasParameter(const std::string& pName, const ParamList& list);

}  // namespace beholder

#endif	// BEHOLDER_CAMERA_PARAM_ENTRY_H
