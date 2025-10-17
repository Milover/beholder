// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BEHOLDER_CAMERA_DEVICE_CLASS_H
#define BEHOLDER_CAMERA_DEVICE_CLASS_H

namespace beholder {
namespace camera {

// Supported transport layer types (device classes).
enum class DeviceClass { Unknown = -1, GigE, Emulated, _max };

}  // namespace camera
}  // namespace beholder

#endif	// BEHOLDER_CAMERA_DEVICE_CLASS_H
