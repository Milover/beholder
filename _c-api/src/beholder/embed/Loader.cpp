// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include "beholder/embed/Loader.h"

#include <dlfcn.h>

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

}  // namespace detail

Loader::Loader(CPathSpan libs, Options opts) noexcept {
	libs_.reserve(libs.size());
	for (const auto& lib : libs) {
		LibHandle handle{detail::dlOpen(lib, enums::to(opts))};
		if (!handle) {
			std::cerr << "failed to load shared object: " << lib << '\n';
			std::exit(EXIT_FAILURE);  // loading cannot fail
		}
		libs_.emplace_back(std::move(handle), lib);
	}
}

}  // namespace embed
}  // namespace beholder
