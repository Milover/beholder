// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include "beholder/camera/Backends.h"

#include <cstdlib>
#include <iostream>

#include "beholder/camera/DeviceClass.h"
#include "beholder/camera/pylon/Camera.h"
#include "beholder/util/Enums.h"

namespace beholder {
namespace camera {

namespace detail {

const ImplEntry::Array BackendFactories{{
	{Backend::Pylon, pylon::createCamera},
}};

}  // namespace detail

detail::CameraImpl::Ptr createCamera(DeviceClass dc, Backend b) noexcept {
	for (const auto& e : detail::BackendFactories) {
		if (e.backend == b) {
			return e.camFactory(dc);
		}
	}
	std::cout << "could not create camera: class = '" << enums::to(dc)
			  << "', backend = '" << enums::to(b) << "'" << std::endl;
	std::exit(EXIT_FAILURE);
}

}  // namespace camera
}  // namespace beholder
