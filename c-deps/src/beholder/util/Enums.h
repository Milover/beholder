// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

// Type traits and helper functions working with enums.

#ifndef BEHOLDER_UTIL_ENUMS_H
#define BEHOLDER_UTIL_ENUMS_H

#include <type_traits>

namespace beholder {
namespace enums {

template<typename T>
using underlying = std::underlying_type_t<T>;

// Enable if T is an enum.
template<typename T, typename U = bool>
using enable_if_enum = std::enable_if_t<std::is_enum_v<T>, U>;

// Check if T is implicitly convertible to E's underlying type.
template<typename T, typename E, bool = std::is_enum_v<E>>
struct is_convertible {
	inline static constexpr bool value =
		std::is_convertible_v<T, underlying<E>>;
};

template<typename T, typename E>
struct is_convertible<T, E, false> : std::false_type {};

template<typename T, typename E>
constexpr bool is_convertible_v = is_convertible<T, E>::value;

// Check if the underlying type of enum E1 can be implicitly converted to
// the underlying type of enum E2, and vice versa.
template<typename E1, typename E2,
		 bool = (std::is_enum_v<E1> && std::is_enum_v<E2>)>
struct is_compatible {
	inline static constexpr bool value =
		std::is_convertible_v<underlying<E1>, underlying<E2>> &&
		std::is_convertible_v<underlying<E2>, underlying<E1>>;
};

template<typename E1, typename E2>
struct is_compatible<E1, E2, false> : std::false_type {};

template<typename E1, typename E2>
constexpr bool is_compatible_v = is_compatible<E1, E2>::value;

// Enable if T is implicitly convertible to E's underlying type.
template<typename T, typename E, typename U = bool>
using enable_if_convertible = std::enable_if_t<is_convertible_v<T, E>, U>;

// Enable if two enums are mutually implicitly convertible.
template<typename E1, typename E2, typename U = bool>
using enable_if_compatible = std::enable_if_t<is_compatible_v<E1, E2>, U>;

// Cast an enum value to a value of a compatible type.
// The enum is cast to it's underlying type if no type is supplied.
template<typename E, enable_if_enum<E> = true>
constexpr underlying<E> to(E e) noexcept {
	return static_cast<underlying<E>>(e);
}
template<typename E, typename T, enable_if_convertible<T, E> = true>
constexpr T to(E e) noexcept {
	return static_cast<T>(e);
}

// Cast a value to an enum, if the value type is implicitly convertible
// to the enum's underlying type.
template<typename E, typename T, enable_if_convertible<T, E> = true>
constexpr E from(T t) noexcept {
	return static_cast<E>(t);
}

}  // namespace enums

// Global addition operator for enum types.
//
// WARNING: no over/underflow checks, yolo.
template<typename E, enums::enable_if_enum<E> = true>
constexpr E operator+(const E& lhs, const E& rhs) noexcept {
	return enums::from<E>(enums::to(lhs) + enums::to(rhs));
}

// Global equality operator for enum types.
//
// No checks for signedness, however, this should be fine since the
// comparison should yield false in case some weird conversions happen.
template<typename E1, typename E2, enums::enable_if_compatible<E1, E2> = true>
constexpr bool operator==(const E1& lhs, const E2& rhs) noexcept {
	using T = std::common_type_t<enums::underlying<E1>, enums::underlying<E2>>;
	return enums::to<E1, T>(lhs) == enums::to<E2, T>(rhs);
};

}  // namespace beholder

#endif	// BEHOLDER_UTIL_ENUMS_H
