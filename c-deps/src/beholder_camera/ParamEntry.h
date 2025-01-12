// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

// Wrapper classes for GenICam parameters.

#ifndef BEHOLDER_CAMERA_PARAM_ENTRY_H
#define BEHOLDER_CAMERA_PARAM_ENTRY_H

#include <string>
#include <vector>

#include "BeholderCameraExport.h"

namespace beholder {

class ParamEntry;
using ParamList = std::vector<ParamEntry>;

// Supported GenICam parameter types.
enum class BH_CAM_API ParamType : int {
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
enum class BH_CAM_API ParamAccessMode { Read, ReadWrite, Unknown = -1 };

// ParamEntry represents a GenICam parameter.
class BH_CAM_API ParamEntry {
public:
	std::string name;					 // parameter name
	std::string value;					 // parameter value
	ParamType type{ParamType::Unknown};	 // parameter type

	// Construct from parts.
	ParamEntry(const std::string& n, const std::string& v, const ParamType& t);

	// Construct from parts.
	ParamEntry(std::string&& n, std::string&& v, ParamType&& t);

	// Construct from name and value.
	ParamEntry(const std::string& n, const std::string& v);

	// Construct from name and value.
	ParamEntry(std::string&& n, std::string&& v);

	ParamEntry() = default;
	ParamEntry(const ParamEntry&) = default;
	ParamEntry(ParamEntry&&) = default;

	~ParamEntry() = default;

	ParamEntry& operator=(const ParamEntry&) = default;
	ParamEntry& operator=(ParamEntry&&) = default;
};

// Convert a GenAPI EInterfaceType to a ParamType.
[[nodiscard]] BH_CAM_API ParamType fromGenAPI(unsigned int i);

// Get a parameter named 'pName' from 'list'.
// Returns an empty ParamEntry if the parameter is not available.
[[nodiscard]] BH_CAM_API ParamEntry getParameter(const std::string& pName,
												 const ParamList& list);

// Check if a parameter named 'pName' is present in 'list'.
BH_CAM_API bool hasParameter(const std::string& pName, const ParamList& list);

}  // namespace beholder

#endif	// BEHOLDER_CAMERA_PARAM_ENTRY_H
