// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BEHOLDER_CAMERA_BACKENDS_H
#define BEHOLDER_CAMERA_BACKENDS_H

#include <array>
#include <memory>

#include "beholder/util/Enums.h"

namespace beholder {
namespace camera {

// Supported camera control backends.
enum class Backend { Pylon, _max };

namespace detail {

class CameraInterface;
class TransportLayerInterface;

template<typename T>
struct Impl {
	using Ptr = std::unique_ptr<T>;
	using Factory = Ptr (*)();
};
using CameraImpl = detail::Impl<CameraInterface>;
using TLImpl = detail::Impl<TransportLayerInterface>;

struct ImplEntry {
	using Array = std::array<ImplEntry, enums::to(Backend::_max)>;

	Backend backend;
	CameraImpl::Factory camFactory;
	TLImpl::Factory tlFactory;
};

extern const ImplEntry::Array Implementations;

}  // namespace detail

[[nodiscard]] detail::CameraImpl::Ptr createCamera(camera::Backend b) noexcept;
[[nodiscard]] detail::TLImpl::Ptr
createTransportLayer(camera::Backend b) noexcept;

}  // namespace camera
}  // namespace beholder

#endif	// BEHOLDER_CAMERA_BACKENDS_H
