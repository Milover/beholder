// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include "beholder/camera/pylon/shim/API.h"

#include <pylon/PylonBase.h>
#include <pylon/TypeMappings.h>

#include <iostream>

#include "beholder/camera/pylon/shim/Export.h"

namespace beholder {
namespace pylonshim {

class BH_PYLONSHIM_API APIImpl : public API {
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

}  // namespace pylonshim
}  // namespace beholder

extern "C" {

BH_PYLONSHIM_API beholder::pylonshim::API* pylonAPI_create() {
	return new beholder::pylonshim::APIImpl{};
}

BH_PYLONSHIM_API void pylonAPI_delete(beholder::pylonshim::API* p) { delete p; }

}  // extern "C"
