// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

// Utility functions for working with the GenICam API.

#ifndef BEHOLDER_CAMERA_INTERNAL_GEN_API_UTILS_H
#define BEHOLDER_CAMERA_INTERNAL_GEN_API_UTILS_H

#include <numeric>
#include <type_traits>
#include <utility>
#include <vector>

#include "beholder/util/Packs.h"

namespace beholder {
namespace internal {

template<
	typename NodeList, typename Node, typename... NodeMaps,
	typename = std::enable_if_t<
		packs::all_same_v<NodeMaps...> &&
		std::is_void_v<
			decltype(std::declval<packs::head_t<packs::pack<NodeMaps...>>>()
						 .GetNodes(std::declval<NodeList&>()))> &&
		std::is_unsigned_v<
			decltype(std::declval<packs::head_t<packs::pack<NodeMaps...>>>()
						 .GetNumNodes())>>>
std::vector<Node> convert(NodeMaps&... nodemaps) {
	using NMap = packs::head_t<packs::pack<NodeMaps...>>;

	std::vector<Node> nodes;
	nodes.reserve((nodemaps.GetNumNodes() + ...));

	NodeList tmp;
	auto collect = [&nodes, &tmp](const NMap m) {
		m.GetNodes(tmp);
		for (auto&& n : tmp) {
			nodes.emplace_back(std::forward<Node>(n));
		}
	};
	(collect(nodemaps), ...);

	return nodes;
}

template<
	typename NodeList, typename Node, typename Condition, typename... NodeMaps,
	typename = std::enable_if_t<
		packs::all_same_v<NodeMaps...> &&
		std::is_void_v<
			decltype(std::declval<packs::head_t<packs::pack<NodeMaps...>>>()
						 .GetNodes(std::declval<NodeList&>()))> &&
		std::is_unsigned_v<
			decltype(std::declval<packs::head_t<packs::pack<NodeMaps...>>>()
						 .GetNumNodes())> &&
		std::is_same_v<std::invoke_result_t<Condition, Node>, bool>>>
std::vector<Node> convert(Condition c, NodeMaps&... nodemaps) {
	using NMap = packs::head_t<packs::pack<NodeMaps...>>;

	std::vector<Node> nodes;
	nodes.reserve((nodemaps.GetNumNodes() + ...));

	NodeList tmp;
	auto collect = [&nodes, &tmp, &c](const NMap& m) {
		m.GetNodes(tmp);
		for (auto&& n : tmp) {
			if (c(n)) {
				nodes.emplace_back(std::forward<Node>(n));
			}
		}
	};
	(collect(nodemaps), ...);

	return nodes;
}

}  // namespace internal
}  // namespace beholder

#endif	// BEHOLDER_CAMERA_INTERNAL_GEN_API_UTILS_H
