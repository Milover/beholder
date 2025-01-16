// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

// A wrapper class for the pylon runtime manager.

#ifndef BEHOLDER_CAMERA_PYLON_API_H
#define BEHOLDER_CAMERA_PYLON_API_H

#include "beholder/BeholderExport.h"

namespace beholder {

// PylonAPI is a helper class to initialize and free resources managed by
// the pylon runtime manager.
//
// WARNING: PylonAPI must be initialized before calling functions defined
// in the 'beholder_camera' library.
//
// NOTE: we could hide this, and just make it get called when anything
// from the library is used, however, this will probably complicate things
// from the Go side --- so we let Go code ensure this gets initialized/freed
// when necessary.
class BH_API PylonAPI {
public:
	// Default constructor.
	// Initializes the pylon runtime manager and resources.
	PylonAPI();

	PylonAPI(const PylonAPI&) = delete;
	PylonAPI(PylonAPI&&) = delete;

	// Destructor.
	// Frees resources managed by the pylon runtime manager.
	~PylonAPI();

	PylonAPI& operator=(const PylonAPI&) = delete;
	PylonAPI& operator=(PylonAPI&&) = delete;
};

}  // namespace beholder

#endif	// BEHOLDER_CAMERA_PYLON_API_H
