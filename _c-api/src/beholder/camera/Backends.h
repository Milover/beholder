// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BEHOLDER_CAMERA_BACKENDS_H
#define BEHOLDER_CAMERA_BACKENDS_H

#include <array>
#include <memory>

#include "beholder/camera/DeviceClass.h"
#include "beholder/util/Enums.h"

namespace beholder {
namespace camera {

class CameraInterface;

// Supported camera control backends.
enum class Backend { Pylon, _max };

namespace detail {

template<typename T>
struct Impl {
	using Type = T;
	using Ptr = std::unique_ptr<T>;
	using Factory = Ptr (*)(DeviceClass);
};
using CameraImpl = detail::Impl<CameraInterface>;

struct ImplEntry {
	using Array = std::array<ImplEntry, enums::to(Backend::_max)>;

	Backend backend;
	CameraImpl::Factory camFactory;
};

extern const ImplEntry::Array BackendFactories;

}  // namespace detail

[[nodiscard]] detail::CameraImpl::Ptr
createCamera(DeviceClass dc, Backend b) noexcept;

}  // namespace camera
}  // namespace beholder

#endif	// BEHOLDER_CAMERA_BACKENDS_H
