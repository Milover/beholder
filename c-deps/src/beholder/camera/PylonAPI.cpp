// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include "beholder/camera/PylonAPI.h"

#include <pylon/PylonBase.h>

namespace beholder {

PylonAPI::PylonAPI() { Pylon::PylonInitialize(); }

PylonAPI::~PylonAPI() { Pylon::PylonTerminate(); }

}  // namespace beholder
