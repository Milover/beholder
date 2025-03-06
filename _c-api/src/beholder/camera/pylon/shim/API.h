// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

// A wrapper class for the pylon runtime manager.

#ifndef BEHOLDER_PYLON_SHIM_API_H
#define BEHOLDER_PYLON_SHIM_API_H

#include "beholder/camera/pylon/shim/Export.h"

namespace beholder {
namespace pylon {
namespace shim {

// API is a helper class to initialize and free resources managed by
// the pylon runtime manager.
//
// WARNING: API must be initialized before calling functions defined
// in the 'beholder_camera' library.
//
// NOTE: we could hide this, and just make it get called when anything
// from the library is used, however, this will probably complicate things
// from the Go side --- so we let Go code ensure this gets initialized/freed
// when necessary.
class BH_PYLON_SHIM_API API {
public:
	// Initializes the pylon runtime manager and resources.
	API() = default;

	API(const API&) = delete;
	API(API&&) = delete;
	API& operator=(const API&) = delete;
	API& operator=(API&&) = delete;

	// Frees resources managed by the pylon runtime manager.
	virtual ~API() noexcept = 0;
};

}  // namespace shim
}  // namespace pylon
}  // namespace beholder

#endif	// BEHOLDER_PYLON_SHIM_API_H
