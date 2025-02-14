// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include "fake/Fake.h"

namespace fake {

// necessary to resolve undefined symbol references
struct FakerImpl : public Faker {
	FakerImpl() = default;
	FakerImpl(const FakerImpl&) = default;
	FakerImpl(FakerImpl&&) = default;

	~FakerImpl() override = default;

	FakerImpl& operator=(const FakerImpl&) = default;
	FakerImpl& operator=(FakerImpl&&) = default;

	int call() override;
};

int FakerImpl::call() { return Return; }  // NOLINT

}  // namespace fake

extern "C" {

int call() { return fake::Return; }
fake::Faker* Faker_Create() { return new fake::FakerImpl{}; }
void Faker_Delete(fake::Faker* ptr) { delete ptr; }

}  // extern "C"
