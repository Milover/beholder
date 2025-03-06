// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include "beholder/camera/pylon/shim/API.h"

#include <pylon/PylonBase.h>
#include <pylon/TypeMappings.h>

#include <iostream>

#include "beholder/camera/pylon/shim/Export.h"

namespace beholder {
namespace pylon {
namespace shim {

class BH_PYLON_SHIM_API APIImpl : public API {
public:
	APIImpl() noexcept;

	APIImpl(const APIImpl&) = delete;
	APIImpl(APIImpl&&) = delete;
	class APIImpl& operator=(const class APIImpl&) = delete;
	class APIImpl& operator=(class APIImpl&&) = delete;

	~APIImpl() noexcept override;
};

APIImpl::APIImpl() noexcept {
	try {
		Pylon::PylonInitialize();
	} catch (const Pylon::GenericException& e) {
		std::cerr << "could not initialize pylon runtime: " << e.what()
				  << std::endl;
	} catch (...) {
		std::cerr << "could not initialize pylon runtime" << std::endl;
	}
}

APIImpl::~APIImpl() noexcept {
	try {
		Pylon::PylonTerminate();
	} catch (const Pylon::GenericException& e) {
		std::cerr << "could not terminate pylon runtime: " << e.what()
				  << std::endl;
	} catch (...) {
		std::cerr << "could not terminate pylon runtime" << std::endl;
	}
}

}  // namespace shim
}  // namespace pylon
}  // namespace beholder

extern "C" {

BH_PYLON_SHIM_API beholder::pylon::shim::API* pylonAPI_create() {
	return new beholder::pylon::shim::APIImpl{};
}

BH_PYLON_SHIM_API void pylonAPI_delete(beholder::pylon::shim::API* p) {
	delete p;
}

}  // extern "C"
