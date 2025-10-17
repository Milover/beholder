// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FAKE_FAKE_H
#define FAKE_FAKE_H

#include "fake/FakeExport.h"

namespace fake {

constexpr static int Return{1337};

struct FAKE_API Faker {
	Faker() = default;
	Faker(const Faker&) = default;
	Faker(Faker&&) = default;

	virtual ~Faker() = default;

	Faker& operator=(const Faker&) = default;
	Faker& operator=(Faker&&) = default;

	virtual int call() = 0;
};

}  // namespace fake

extern "C" {

FAKE_API int call();
FAKE_API fake::Faker* Faker_Create();
FAKE_API void Faker_Delete(fake::Faker* ptr);

}  // extern "C"

#endif	// FAKE_FAKE_H
