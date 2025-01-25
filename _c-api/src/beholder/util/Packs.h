// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

// Type sequence (parameter pack) helpers.
//
// TODO: should probably refactor the latter portion to use pack, not
// raw parameter packs.

#ifndef BEHOLDER_UTIL_PACKS_H
#define BEHOLDER_UTIL_PACKS_H

#include <type_traits>

namespace beholder {
namespace packs {

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

// A type to check if all types in a sequence satisfy a given unary predicate.
template<template<typename> typename Pred, typename... Ts>
struct all_of {
	inline static constexpr bool value = (Pred<Ts>::value && ...);
};

template<template<typename> typename Pred, typename... Ts>
constexpr bool all_of_v = all_of<Pred, Ts...>::value;

// A type to check if all types in a sequence satisfy a given binary predicate,
// where T is the first argument to the binary predicate.
template<template<typename, typename> typename Pred, typename T, typename... Ts>
struct binary_all_of {
	inline static constexpr bool value = (Pred<T, Ts>::value && ...);
};

template<template<typename, typename> typename Pred, typename T, typename... Ts>
constexpr bool binary_all_of_v = binary_all_of<Pred, T, Ts...>::value;

// A type to check if the type sequence is uniform/homogeneous (consists of
// only one type).
template<typename... Ts>
struct all_same;

template<typename T, typename... Ts>
struct all_same<T, Ts...> {
	inline static constexpr bool value =
		binary_all_of_v<std::is_same, T, Ts...>;
};

template<typename... Ts>
constexpr bool all_same_v = all_same<Ts...>::value;

}  // namespace packs
}  // namespace beholder

#endif	// BEHOLDER_UTIL_PACK_H
