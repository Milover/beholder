// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

// Camera device class definitions.

#ifndef BEHOLDER_CAMERA_CAMERA_H
#define BEHOLDER_CAMERA_CAMERA_H

#include <cstddef>
#include <optional>
#include <string>

#include "beholder/camera/Backends.h"
#include "beholder/camera/Config.h"
#include "beholder/camera/Parameter.h"
#include "beholder/camera/TriggerType.h"
#include "beholder/capi/Image.h"

namespace beholder {
namespace camera {

// Camera represents a physical camera device.
class Camera {
private:
	using CamPtr = detail::CameraImpl::Ptr;

	Config cfg_;   // camera configuration
	CamPtr impl_;  // underlying camera implementation

public:
	// Default constructor.
	// The camera must be initialized with Camera::init before use.
	explicit Camera(Config cfg = Config{});

	// Initialize the camera, optionally (re)setting the device designator.
	// Returns true if there are no errors and the device is attached and
	// open after initialization, otherwise returns false.
	[[nodiscard]] bool init() noexcept;
	[[nodiscard]] bool init(const std::string& designator) noexcept;
	[[nodiscard]] bool init(const char* designator) noexcept;

	// Get camera parameters
	[[nodiscard]] ParamVector getParams(
		Parameter::AccessMode mode = Parameter::AccessMode::ReadWrite) noexcept;

	// Set camera parameters in the order provided.
	// Returns true if no errors ocurred.
	bool setParams(CParamSpan params) noexcept;

	// Execute a trigger.
	bool trigger(TriggerType typ = TriggerType::Software) noexcept;

	// Waits for the trigger to become ready and then executes the trigger.
	bool waitAndTrigger(TriggerType typ = TriggerType::Software) noexcept;

	// Acquire an image.
	// WARNING: acquisition must be started manually, however,
	// acquisition can be stopped automatically, eg. when a certain
	// number of images has been acquired.
	bool acquire() noexcept;

	// Start image acquisition and stop after nImages have been acquired.
	// If nImages is 0, the camera will keep acquiring indefinitely.
	bool startAcquisition(size_t nImages = 0UL) noexcept;

	// Stop image acquisition.
	void stopAcquisition() noexcept;

	// Return acquisition state.
	[[nodiscard]] bool isAcquiring() const noexcept;

	// Check if the camera is initialized (device attached and open).
	[[nodiscard]] bool isInitialized() const noexcept;

	// Check if the camera device is attached.
	[[nodiscard]] bool isAttached() const noexcept;

	// Get the acquired result as a raw image.
	[[nodiscard]] std::optional<Image> getImage() noexcept;

	// Execute a GenICam command on the camera device.
	// Returns false if there was an error.
	bool cmdExecute(const std::string& cmd) noexcept;
	bool cmdExecute(const char* cmd) noexcept;

	// Report if command execution finished.
	[[nodiscard]] bool cmdIsDone(const std::string& cmd) noexcept;
	[[nodiscard]] bool cmdIsDone(const char* cmd) noexcept;
};

}  // namespace camera
}  // namespace beholder

#endif	// BEHOLDER_CAMERA_CAMERA_H
