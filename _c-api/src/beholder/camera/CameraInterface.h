// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

// Camera device class definitions.

#ifndef BEHOLDER_CAMERA_CAMERA_INTERFACE_H
#define BEHOLDER_CAMERA_CAMERA_INTERFACE_H

#include <chrono>
#include <cstddef>
#include <optional>

#include "beholder/camera/Parameter.h"
#include "beholder/camera/TriggerType.h"
#include "beholder/capi/Image.h"

namespace beholder {
namespace camera {

// CameraInterface is a base class for the underlying camera implementations.
class CameraInterface {
public:
	using Milliseconds = std::chrono::milliseconds;

	CameraInterface() = default;
	CameraInterface(const CameraInterface&) = delete;
	CameraInterface(CameraInterface&&) = delete;
	CameraInterface& operator=(const CameraInterface&) = delete;
	CameraInterface& operator=(CameraInterface&&) = delete;
	virtual ~CameraInterface() noexcept;

	[[nodiscard]] virtual bool init(const char* designator,
									Milliseconds timeout,
									bool reboot) noexcept = 0;

	[[nodiscard]] virtual ParamVector
	getParams(Parameter::AccessMode mode) noexcept = 0;
	virtual bool setParams(CParamSpan params) noexcept = 0;

	virtual bool trigger(TriggerType typ) noexcept = 0;
	virtual bool
	waitAndTrigger(Milliseconds timeout, TriggerType typ) noexcept = 0;

	virtual bool acquire(Milliseconds timeout) noexcept = 0;
	virtual bool startAcquisition(size_t nImages) noexcept = 0;
	virtual void stopAcquisition() noexcept = 0;

	[[nodiscard]] virtual bool isAcquiring() const noexcept = 0;
	[[nodiscard]] virtual bool isAttached() const noexcept = 0;
	[[nodiscard]] virtual bool isInitialized() const noexcept = 0;

	[[nodiscard]] virtual std::optional<Image> getImage() noexcept = 0;

	virtual bool cmdExecute(const char* cmd) noexcept = 0;
	[[nodiscard]] virtual bool cmdIsDone(const char* cmd) noexcept = 0;
};

}  // namespace camera
}  // namespace beholder

#endif	// BEHOLDER_CAMERA_CAMERA_INTERFACE_H
