// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include "beholder/camera/Backends.h"

#include <cstdlib>
#include <iostream>

#include "beholder/camera/Config.h"
#include "beholder/camera/pylon/Camera.h"
#include "beholder/util/Enums.h"

namespace beholder {
namespace camera {

namespace detail {

const ImplEntry::Array BackendFactories{{
	{Backend::Pylon, pylon::createCamera},
}};

}  // namespace detail

detail::CameraImpl::Ptr createCamera(const Config& cfg) noexcept {
	for (const auto& e : detail::BackendFactories) {
		if (e.backend == cfg.backend) {
			return e.camFactory(cfg);
		}
	}
	std::cout << "could not create camera: class = '"
			  << enums::to(cfg.deviceClass) << "', backend = '"
			  << enums::to(cfg.backend) << "'" << std::endl;
	std::exit(EXIT_FAILURE);
}

}  // namespace camera
}  // namespace beholder
