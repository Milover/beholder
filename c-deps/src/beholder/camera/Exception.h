// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

// A generic exception class which simplifies re-throwing/wrapping
// other exceptions.
//
// TODO: this class should probably get removed once 'Result' gets properly
// implemented.

#ifndef BEHOLDER_CAMERA_EXCEPTION_H
#define BEHOLDER_CAMERA_EXCEPTION_H

#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include "beholder/BeholderExport.h"

namespace beholder {

class BH_API Exception : public std::runtime_error {
public:
	using Base = std::runtime_error;

	// Construct from a string description.
	explicit Exception(const std::string& what_arg) : Base(what_arg) {}

	// Construct from a char* description.
	explicit Exception(const char* what_arg) : Base(what_arg) {}

	// Construct from a string description and another exception 'e',
	// by appending 'e.what()' to the description.
	template<typename E, typename = std::enable_if_t<std::is_invocable_r_v<
							 const char*, decltype(&E::what), E&> > >
	Exception(const std::string& what_arg, const E& e)
		: Base(what_arg + e.what()) {}

	// Construct from a char* description and another exception 'e',
	// by appending 'e.what()' to the description.
	template<typename E, typename = std::enable_if_t<std::is_invocable_r_v<
							 const char*, decltype(&E::what), E&> > >
	Exception(const char* what_arg, const E& e)
		: Base(std::string{what_arg} + e.what()) {}

	Exception(const Exception&) = default;
	Exception(Exception&&) = default;

	Exception& operator=(const Exception&) = default;
	Exception& operator=(Exception&&) = default;

	~Exception() override = default;
};

}  // namespace beholder

#endif	// BEHOLDER_CAMERA_EXCEPTION_H
