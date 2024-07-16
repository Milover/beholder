/*---------------------------------------------------------------------------*\

	camera - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A wrapper class for parameter types.

SourceFiles
	ParamEntry.cpp

\*---------------------------------------------------------------------------*/

#ifndef CAMERA_PARAM_ENTRY_H
#define CAMERA_PARAM_ENTRY_H

#include <string>
#include <vector>

#include <pylon/PylonIncludes.h>

// * * * * * * * * * * * * * Forward Declarations  * * * * * * * * * * * * * //

namespace camera
{
	class ParamEntry;
	using ParamList = std::vector<ParamEntry>;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace camera
{

enum class ParamType : int
{
	Int = GenApi::intfIInteger,
	Bool = GenApi::intfIBoolean,
	Command = GenApi::intfICommand,
	Float = GenApi::intfIFloat,
	String = GenApi::intfIString,
	Enum = GenApi::intfIEnumeration,
	Register = GenApi::intfIRegister,
	Unknown = -1	// we don't actually know if this is ok; yolo
};

/*---------------------------------------------------------------------------*\
                        Class ParamEntry Declaration
\*---------------------------------------------------------------------------*/


class ParamEntry
{
public:

	// Public data

		std::string name;
		std::string value;
		ParamType type {ParamType::Unknown};

	// Constructors

		// Default constructor
		ParamEntry() = default;

		// Construct from parts
		ParamEntry
		(
			const std::string& n,
			const std::string& v,
			const ParamType& t
		);

		// Construct from parts
		ParamEntry(std::string&& n, std::string&& v, ParamType&& t);

		// Construct from name and value
		ParamEntry(const std::string& n, const std::string& v);

		// Construct from name and value
		ParamEntry(std::string&& n, std::string&& v);

		// Construct from a copy
		ParamEntry(const ParamEntry& pe) = default;

	// Destructor
	~ParamEntry() = default;

	// Member functions


};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

// Convert a GenApi::EInterfaceType to a ParamType.
ParamType convertInterfaceType(GenApi::EInterfaceType e);

// Get a parameter named 'pName' from 'list'.
// Returns an empty ParamEntry if the parameter is not available.
ParamEntry getParameter(const std::string& pName, const ParamList& list);

// Check if a parameter named 'pName' is present in 'list'.
bool hasParameter(const std::string& pName, const ParamList& list);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace camera

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
