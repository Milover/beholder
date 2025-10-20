// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BEHOLDER_UTIL_SCOPE_GUARD_H
#define BEHOLDER_UTIL_SCOPE_GUARD_H

#include <type_traits>
#include <utility>

namespace beholder {

// ScopeGuard is a simple type for wrapping a resource-cleanup call.
// Use it like so:
//
// 	int main()
// 	{
// 		auto f = []{};
// 		ScopeGuard g(f);
// 		ScopeGuard h([]{});
// 	}
//
// https://stackoverflow.com/a/61242721
template<typename F>
struct ScopeGuard {
	static_assert(std::is_nothrow_invocable_r_v<void, F>);

	template<typename U>
	explicit ScopeGuard(U&& f) : func{std::forward<U>(f)} {}

	ScopeGuard(const ScopeGuard&) = delete;
	ScopeGuard(ScopeGuard&&) = delete;

	~ScopeGuard() noexcept { func(); }

	ScopeGuard& operator=(const ScopeGuard&) = delete;
	ScopeGuard& operator=(ScopeGuard&&) = delete;

	F func;
};

template<typename F>
ScopeGuard(F&& f) -> ScopeGuard<F>;

}  // namespace beholder

#endif	// BEHOLDER_UTIL_SCOPE_GUARD_H
