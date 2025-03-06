// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include "beholder/camera/Backends.h"

#include <cstdlib>

#include "beholder/camera/pylon/Camera.h"
#include "beholder/camera/pylon/TransportLayer.h"

namespace beholder {
namespace camera {

namespace detail {

const ImplEntry::Array Implementations{{
	{Backend::Pylon, pylon::createCamera, pylon::createTransportLayer},
}};

}  // namespace detail

detail::CameraImpl::Ptr createCamera(camera::Backend b) noexcept {
	for (const auto& e : detail::Implementations) {
		if (e.backend == b) {
			return e.camFactory();
		}
	}
	std::exit(EXIT_FAILURE);
}

detail::TLImpl::Ptr createTransportLayer(camera::Backend b) noexcept {
	for (const auto& e : detail::Implementations) {
		if (e.backend == b) {
			return e.tlFactory();
		}
	}
	std::exit(EXIT_FAILURE);
}

}  // namespace camera
}  // namespace beholder
