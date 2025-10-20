// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

// Wrapper classes for GenICam parameters.

#ifndef BEHOLDER_CAMERA_PARAMETER_H
#define BEHOLDER_CAMERA_PARAMETER_H

#include <span>
#include <string>
#include <utility>
#include <vector>

namespace beholder {
namespace camera {

class Parameter;
using ParamVector = std::vector<Parameter>;
using CParamSpan = std::span<const Parameter>;

// Parameter represents a GenICam parameter.
class Parameter {
public:
	// Supported parameter access modes.
	enum class AccessMode { Unknown = -1, Read, ReadWrite };

	// Supported GenICam parameter types.
	enum class Type {
		Unknown = -1,  // we don't actually know if this is ok; yolo
		_val,		   // GenAPI value (unused)
		_base,		   // GenAPI base (unused)
		Int,		   // GenAPI integer
		Bool,		   // GenAPI boolean
		Cmd,		   // GenAPI command
		Float,		   // GenAPI float
		Str,		   // GenAPI string
		Reg,		   // GenAPI register
		_cat,		   // GenAPI category (unused)
		Enum,		   // GenAPI enumeration
		_entry,		   // GenAPI enumeration entry (unused)
		_port,		   // GenAPI port (unused)
	};

	std::string name;		   // parameter name
	std::string value;		   // parameter value
	Type type{Type::Unknown};  // parameter type

	Parameter() = default;

	Parameter(std::string n, std::string v, Type t = Type{Type::Unknown})
		: name{std::move(n)}, value{std::move(v)}, type{t} {}
};

// Get the first parameter with the name pName from params.
// Returns an empty Parameter if the parameter is not found.
[[nodiscard]] Parameter
getParameter(const std::string& pName, CParamSpan params);

// Check if a parameter with the name pName is present in params.
[[nodiscard]] bool hasParameter(const std::string& pName, CParamSpan params);

}  // namespace camera
}  // namespace beholder

#endif	// BEHOLDER_CAMERA_PARAMETER_H
