// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#include "PylonAPI.h"

#include <pylon/PylonBase.h>

namespace beholder {

PylonAPI::PylonAPI() { Pylon::PylonInitialize(); }

PylonAPI::~PylonAPI() { Pylon::PylonTerminate(); }

}  // namespace beholder
