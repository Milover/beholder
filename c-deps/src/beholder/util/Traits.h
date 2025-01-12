// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

// Handy-dandy type traits.

#ifndef BEHOLDER_UTIL_TRAITS_H
#define BEHOLDER_UTIL_TRAITS_H

#include <type_traits>

namespace beholder {

template<typename T>
using remove_cvref_t = std::remove_reference_t<std::remove_cv_t<T>>;

template<typename T, typename U>
using enable_if_same_t = std::enable_if_t<std::is_same_v<remove_cvref_t<T>, U>>;

// Type traits for working with enums.
namespace enums {

template<typename T>
using underlying = std::underlying_type_t<T>;

// Check if T is implicitly convertbile to E's underlying type.
template<typename T, typename E>
inline constexpr bool is_convertible =
	std::is_enum_v<E> && std::is_convertible_v<T, underlying<E>>;

// Check if T is implicitly no-throw convertbile to E's underlying type.
template<typename T, typename E>
inline constexpr bool is_nothrow_convertible =
	std::is_enum_v<E> && std::is_nothrow_convertible_v<T, underlying<E>>;

template<typename T, typename U = bool>
using enable_if_enum = std::enable_if_t<std::is_enum_v<T>, U>;

template<typename T, typename E, typename U = bool>
using enable_if_convertible = std::enable_if_t<is_convertible<T, E>, U>;

template<typename T, typename E, typename U = bool>
using enable_if_nothrow_convertible =
	std::enable_if_t<is_nothrow_convertible<T, E>, U>;

// Cast an enum value to a value of a compatible type.
// The enum is cast to it's underlying type by default.
template<typename E, typename T = underlying<E>,
		 enable_if_convertible<T, E> = true>
constexpr T to(E e) noexcept(is_nothrow_convertible<T, E>) {
	return static_cast<T>(e);
}

// Cast a value to an enum, if the value type is implicitly convertible
// to the enum's underlying type.
template<typename E, typename T, enable_if_convertible<T, E> = true>
constexpr E from(T t) noexcept(is_nothrow_convertible<T, E>) {
	return static_cast<E>(t);
}

}  // namespace enums

// Type sequence (parameter pack) helpers.
namespace pack {
// A type to hold a parameter pack
template<typename... Ts>
struct pack {};

// A type to return the first type from a type sequence (parameter pack)
template<typename... Ts>
struct head;

template<typename T, typename... Ts>
struct head<pack<T, Ts...>> {
	using type = T;
};

template<typename T>
using head_t = typename head<T>::type;

// A type to return all types of a type sequence (parameter pack),
// after the first type
template<typename... Ts>
struct tail;

template<typename T, typename... Ts>
struct tail<pack<T, Ts...>> {
	using type = pack<Ts...>;
};

template<typename T>
using tail_t = typename tail<T>::type;

// A type to prepend a new type to a type sequence (parameter pack)
template<typename T, typename Seq>
struct cons;

template<typename T, typename... Ts>
struct cons<T, pack<Ts...>> {
	using type = pack<T, Ts...>;
};

template<typename T, typename Seq>
using cons_t = typename cons<T, Seq>::type;

// A type to check if the type sequence is uniform/homogeneous (consists of
// only one type).
template<typename... Ts>
struct is_uniform : std::false_type {};

template<typename T, typename... Ts>
struct is_uniform<T, Ts...>
	: std::bool_constant<(std::is_same_v<T, Ts> && ...)> {};

template<typename... Ts>
inline static constexpr bool is_uniform_v = is_uniform<Ts...>::value;

}  // namespace pack
}  // namespace beholder

#endif	// BEHOLDER_UTIL_TRAITS_H
