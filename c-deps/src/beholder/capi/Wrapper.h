// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

// A base class template to simplify wrapping C-structs.

#ifndef BEHOLDER_CAPI_WRAPPER_H
#define BEHOLDER_CAPI_WRAPPER_H

#ifdef __cplusplus
#include <type_traits>
#include <utility>

#include "beholder/util/Traits.h"

namespace beholder {
namespace capi {

namespace detail {
template<typename T>
struct DfltCtor {
	T operator()() { return T{}; }
};

template<typename T>
struct DfltDtor {
	void operator()([[maybe_unused]] T& t) {}
};
}  // namespace detail

template<
	typename T, typename Ctor = detail::DfltCtor<T>,
	typename Dtor = detail::DfltDtor<T>,
	std::enable_if_t<std::is_trivial_v<T> && std::is_standard_layout_v<T> &&
						 std::is_default_constructible_v<Ctor> &&
						 std::is_invocable_r_v<T, Ctor> &&
						 std::is_default_constructible_v<Dtor> &&
						 std::is_invocable_r_v<void, Dtor, T&>,
					 bool> = true>
class Wrapper {
protected:
	T t_;  // the underlying C struct

public:
	using type = T;

	// Default constructor
	Wrapper() : t_{Ctor{}()} {}

	// Construct by forwarding arguments to T's constructor.
	template<typename... Args>
	explicit Wrapper(Args&&... args) : t_{std::forward<Args>(args)...} {}

	// Forwarding constructor from a T.
	template<typename U, typename = enable_if_same_t<U, T>>
	explicit Wrapper(U&& t) : t_{std::forward<U>(t)} {}

	// Default copy constructor.
	Wrapper(const Wrapper&) = default;

	// Default move constructor.
	Wrapper(Wrapper&&) noexcept = default;

	// Destructor.
	virtual ~Wrapper() { Dtor{}(t_); }

	// Return a reference to the underlying T.
	virtual T& ref() { return t_; }

	// Return a const reference to the underlying T.
	virtual const T& cRef() const { return t_; }

	// Return a copy of the underlying T.
	virtual T toC() const { return t_; }

	// Move the underlying T out.
	virtual T&& moveToC() && { return std::move(t_); }

	// Implicit conversion to a T&
	//operator T&() { return t_; }

	// Implicit conversion to a const T&
	//operator const T&() const { return t_; }

	// Default copy assignment operator.
	Wrapper& operator=(const Wrapper&) = default;

	// Default move assignment operator.
	Wrapper& operator=(Wrapper&&) noexcept = default;
};

}  // namespace capi
}  // namespace beholder

#endif	// __cplusplus

#endif	// BEHOLDER_CAPI_WRAPPER_H
