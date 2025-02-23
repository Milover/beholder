// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BEHOLDER_EMBED_LOADER_H
#define BEHOLDER_EMBED_LOADER_H

#include <dlfcn.h>

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <memory>
#include <span>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "beholder/util/Enums.h"

namespace beholder {
namespace embed {

namespace detail {
struct DLCloser;
}

using CPathSpan = std::span<const std::filesystem::path>;
using PathVector = std::vector<std::filesystem::path>;
using LibHandle = std::unique_ptr<void, detail::DLCloser>;
using LibVector = std::vector<std::pair<LibHandle, std::filesystem::path>>;

namespace detail {

struct DLCloser {
	void operator()(void* handle) noexcept;
};

// Unload a dynamically loaded shared object.
//
// The object is not guaranteed to be unloaded after dlClose returns.
//
// This is intended for internal use. Prefer using dlOpen and relying on the
// provided deleter to handle unloading properly.
void dlClose(void* handle) noexcept;

// Dynamically load a shared object.
//
// The returned object handle's deleter takes care of unloading the object.
LibHandle
dlOpen(const std::filesystem::path& p, int flags = RTLD_LAZY) noexcept;

// Load a symbol from a dynamically loaded shared object.
template<typename T, std::enable_if_t<std::is_pointer_v<T>, bool> = true>
T dlSym(void* handle, const std::string& sym) noexcept {
	dlerror();	// clear errors
	if (handle == nullptr) {
		return nullptr;
	}
	// the symbol might legitimately be NULL, so this is the
	// recommended way of checking for errors
	void* s{dlsym(handle, sym.c_str())};
	const char* err{dlerror()};
	if (err != nullptr) {
		std::cerr << err << '\n';
	}
	return reinterpret_cast<T>(s);	// NOLINT
}

// isDLOpen checks if lib has been dynamically loaded.
// Returns false if lib is empty.
bool isDLOpen(const std::filesystem::path& lib) noexcept;

}  // namespace detail

// NOTE: providing open() and close() complicates things since we allow
// for the possibility of an object being opened/closed multiple times,
// which we don't want to support (and don't need).
//
// The point of the Loader is to simplify handling dynamic objects and restrict
// the API, not handle all possible use-cases (we just have to load all our
// stuff at start-up, and clean up at exit).
class Loader {
private:
	LibVector libs_;  // currently loaded libraries

	// findLibPredicate is a helper function which used when searching for
	// a library managed by Loader.
	//
	// The path lib matches a managed library if any of the following are true:
	// 	1. lib is exactly equal to a library path
	// 	2. lib is exactly equal to a file name component of a library path
	// 	3. lib is exactly equal to a file name component without the final
	// 	   extension of a library path
	//
	// Note that (3) only considers the final extension, for example:
	// 'libmylib' will match '/some/path/libmylib.so', but not
	// '/some/path/libmylib.so.1.0'.
	[[nodiscard]] static bool
	findLibPredicate(const std::filesystem::path& p,
					 const std::filesystem::path& lib) noexcept {
		// NOTE: we should think whether we want to require the fully
		// qualified name of the library, that is, exactly the name which
		// was used to load the library, in order for this to return true
		return p == lib || p.filename() == lib || p.stem() == lib;
	}

	[[nodiscard]] LibVector::const_iterator
	findLib(const std::filesystem::path& lib) const noexcept {
		return std::ranges::find_if(libs_, [&lib](const auto& t) {
			return findLibPredicate(t.second, lib);
		});
	}

	[[nodiscard]] LibVector::iterator
	findLib(const std::filesystem::path& lib) noexcept {
		return std::ranges::find_if(libs_, [&lib](const auto& t) {
			return findLibPredicate(t.second, lib);
		});
	}

public:
	// See man dlopen(3) for more info.
	enum class Options : int {
		Lazy = RTLD_LAZY,
		Now = RTLD_NOW,
		Global = RTLD_GLOBAL,
		Local = RTLD_LOCAL,
		NoDelete = RTLD_NODELETE,
		NoLoad = RTLD_NOLOAD,
		DeepBind = RTLD_DEEPBIND,
	};

	// Construct by loading shared objects in the provided order.
	//
	// If there is an error, std::exit is called with EXIT_FAILURE.
	//
	// TODO: could also enable per-object flags
	explicit Loader(CPathSpan libs,
					Options opts = Options::Lazy | Options::Local) noexcept;

	Loader(const Loader&) = delete;
	Loader(Loader&&) = default;
	Loader& operator=(const Loader&) = delete;
	Loader& operator=(Loader&&) = default;

	// Closes all shared objects currently managed by this instance.
	//
	// NOTE: objects are closed only once, so if there are multiple Loaders
	// handling the same object, the object might still be loaded after
	// the destructor returns.
	~Loader() = default;

	// TODO: add getter for loaded library paths

	// matchPaths constructs a list of library paths by selecting paths
	// corresponding to libs. Matching is done using Loader::findLibPredicate.
	//
	// If force is set to true, matching errors will cause the program to
	// terminate, otherwise, matching errors are reported, but ignored.
	//
	// NOTE: pulling out load-order into a separate class might be beneficial
	[[nodiscard]] static PathVector
	matchPaths(CPathSpan paths, CPathSpan libs, bool force = true) noexcept;

	// pathTo returns the full path to lib, if lib has been loaded.
	// Matching is done using Loader::findLibPredicate.
	[[nodiscard]] std::filesystem::path
	pathTo(const std::filesystem::path& lib) const noexcept;

	// getSymbol tries to load the symbol sym from the shared object lib.
	// If lib is loaded and sym is found, returns a handle to sym of type T.
	template<typename T, std::enable_if_t<std::is_pointer_v<T>, bool> = true>
	[[nodiscard]] T getSymbol(const std::filesystem::path& lib,
							  const std::string& sym) const noexcept {
		const LibVector::const_iterator it{findLib(lib)};
		if (it == libs_.end()) {
			return nullptr;
		}
		return detail::dlSym<T>(it->first.get(), sym.c_str());
	}

	// getClass returns a managed pointer to an object of type T.
	//
	// Returns a std::unique_ptr<T, Del>, where Del is a wrapper for
	// T's destructor function dtor, initialized by calling T's constructor
	// function ctor, or nullptr if there is an error.
	//
	// NOTE: ctorSym must be a symbol of a function of type T*(), and
	// and dtorSym must be a symbol of a function of type void(T*).
	template<typename T>
	[[nodiscard]] auto
	getClass(const std::filesystem::path& lib, const std::string& ctorSym,
			 const std::string& dtorSym) const noexcept {
		using TPtr = std::add_pointer_t<T>;
		using Ctor = TPtr (*)();
		using Dtor = void (*)(TPtr);
		struct Del {
			Dtor del;
			explicit Del(Dtor d) : del{d} {}
			void operator()(TPtr ptr) { del(ptr); };
		};
		using Ret = std::unique_ptr<T, Del>;

		const LibVector::const_iterator it{findLib(lib)};
		if (it == libs_.end()) {
			return Ret{nullptr, Del{nullptr}};
		}
		void* handle{it->first.get()};
		Ctor ctor{detail::dlSym<Ctor>(handle, ctorSym.c_str())};
		Dtor dtor{detail::dlSym<Dtor>(handle, dtorSym.c_str())};
		if (!ctor || !dtor) {
			return Ret{nullptr, Del{nullptr}};
		}
		return Ret{ctor(), Del{dtor}};
	}
};

}  // namespace embed
}  // namespace beholder

#endif	// BEHOLDER_EMBED_LOADER_H
