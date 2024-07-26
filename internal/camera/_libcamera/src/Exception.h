/*---------------------------------------------------------------------------*\

	camera - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A generic exception class which simplifies re-throwing/wrapping
	other exceptions.

\*---------------------------------------------------------------------------*/

#ifndef CAMERA_EXCEPTION_H
#define CAMERA_EXCEPTION_H

#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace camera
{

/*---------------------------------------------------------------------------*\
                          Class Exception Definition
\*---------------------------------------------------------------------------*/

class Exception
:
	public std::runtime_error
{
public:

	using Base = std::runtime_error;

	// Constructors

		//- Construct from a string description
		Exception(const std::string& what_arg)
		:
			Base(what_arg)
		{}

		//- Construct from a char* description
		Exception(const char* what_arg)
		:
			Base(what_arg)
		{}

		//- Construct from a string description and another exception 'e',
		//	by appending 'e.what()' to the description.
		template
		<
			typename E,
			typename = std::enable_if_t
			<
				std::is_invocable_r_v<const char*, decltype(&E::what), E&>
			>
		>
		Exception(const std::string& what_arg, const E& e)
		:
			Base(what_arg + e.what())
		{}

		//- Construct from a char* description and another exception 'e',
		//	by appending 'e.what()' to the description.
		template
		<
			typename E,
			typename = std::enable_if_t
			<
				std::is_invocable_r_v<const char*, decltype(&E::what), E&>
			>
		>
		Exception(const char* what_arg, const E& e)
		:
			Base(std::string {what_arg} + e.what())
		{}

		//- Default copy constructor
		Exception(const Exception&) = default;

	//- Destructor
	virtual ~Exception() = default;

	// Member operators

		//- Default copy assignment
		Exception& operator=(const Exception&) = default;

};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace camera

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
