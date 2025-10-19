// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include "beholder/embed/Loader.h"

#include <dlfcn.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <utility>

#include "beholder/util/Enums.h"

namespace fs = std::filesystem;

namespace beholder {
namespace embed {

namespace detail {

void DLCloser::operator()(void* handle) noexcept { dlClose(handle); }

LibHandle dlOpen(const fs::path& p, int flags) noexcept {
	dlerror();	// clear errors
	void* handle{dlopen(p.c_str(), flags)};
	// this is more robust than relying on the return value, for example
	// a NULL return with RTLD_NOLOAD might not be an error
	const char* err{dlerror()};
	if (err != nullptr) {
		std::cerr << err << '\n';
	}
	return LibHandle{handle, DLCloser{}};
}

void dlClose(void* handle) noexcept {
	if (handle == nullptr) {
		return;
	}
	const int rc{dlclose(handle)};
	if (rc != 0) {
		std::cerr << dlerror() << '\n';
	}
}

bool isDLOpen(const std::filesystem::path& lib) noexcept {
	if (lib.empty()) {
		return false;
	}
	const embed::LibHandle handle{
		embed::detail::dlOpen(lib, RTLD_LAZY | RTLD_NOLOAD)};
	return static_cast<bool>(handle);
}

}  // namespace detail

Loader::Loader(CPathSpan libs, Options opts) noexcept {
	libs_.reserve(libs.size());
	for (const auto& lib : libs) {
		std::cerr << "loading: " << lib << std::endl;
		LibHandle handle{detail::dlOpen(lib, enums::to(opts))};
		if (!handle) {
			std::cerr << "failed to load shared object: " << lib << '\n';
			std::exit(EXIT_FAILURE);  // loading cannot fail
		}
		libs_.emplace_back(std::move(handle), lib);
	}
}

Loader::~Loader() noexcept {
	for (auto it{libs_.rbegin()}; it != libs_.rend(); ++it) {
		it->handle.reset();
		if (detail::isDLOpen(it->path)) {
			std::cerr << "FAILED: unloading: "  << it->path << '\n';
		} else {
			std::cerr << "OK: unloaded: "  << it->path << '\n';
		}
	}
}

// NOLINTBEGIN(bugprone-easily-swappable-parameters)
PathVector
Loader::matchPaths(CPathSpan paths, CPathSpan libs, bool force) noexcept {
	PathVector res;
	res.reserve(libs.size());
	for (const auto& l : libs) {
		auto found{std::ranges::find_if(paths, [&l](const auto& v) {
			return Loader::findLibPredicate(v, l);
		})};
		if (found == paths.end()) {
			std::cerr << "failed to match library to path: " << l << '\n';
			if (force) {
				std::exit(EXIT_FAILURE);
			}
		}
		res.emplace_back(*found);
	}
	return res;
}
// NOLINTEND(bugprone-easily-swappable-parameters)

fs::path Loader::pathTo(const fs::path& lib) const noexcept {
	auto found{findLib(lib)};
	if (found == libs_.end()) {
		return fs::path{};
	}
	return found->path;
}

}  // namespace embed
}  // namespace beholder
