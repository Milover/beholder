// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FAKE_FAKE_H
#define FAKE_FAKE_H

#include "fake/FakeExport.h"

namespace fake {

FAKE_API constexpr static int Return{1337};

}  // namespace fake

extern "C" FAKE_API int call();

#endif	// FAKE_FAKE_H
