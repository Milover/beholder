// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

// Camera device class definitions.

#ifndef BEHOLDER_PYLONSHIM_CAMERA_H
#define BEHOLDER_PYLONSHIM_CAMERA_H

#include <chrono>
#include <cstddef>
#include <optional>
#include <span>
#include <vector>

#include "beholder/camera/Parameter.h"
#include "beholder/camera/pylon/shim/Export.h"
#include "beholder/capi/Image.h"

namespace Pylon {
class IPylonDevice;
}  // namespace Pylon

namespace beholder {
namespace pylonshim {

// Camera represents a physical camera device.
class BH_PYLONSHIM_API Camera {
public:
	using Parameter = beholder::camera::Parameter;
	using CParamSpan = std::span<const Parameter>;
	using ParamVector = std::vector<Parameter>;

protected:
	// Default constructor.
	// The camera must be initialized with Camera::init before use.
	Camera() = default;

public:
	Camera(const Camera&) = delete;
	Camera(Camera&&) = default;
	Camera& operator=(const Camera&) = delete;
	Camera& operator=(Camera&&) = default;

	// Default destructor.
	// Defined in the source because unique_ptr complains about
	// incomplete types.
	virtual ~Camera() = default;

	// Initialize camera device.
	// The device is attached and open after initialization.
	//
	// NOTE: takes ownership of the supplied device.
	[[nodiscard]] virtual bool init(Pylon::IPylonDevice* d) noexcept = 0;

	// Get camera parameters
	[[nodiscard]] virtual ParamVector getParams(Parameter::AccessMode mode) = 0;

	// Set camera parameters in the order provided.
	// Returns true if no errors ocurred.
	virtual bool setParams(CParamSpan params) noexcept = 0;

	// Execute a trigger.
	virtual bool trigger() noexcept = 0;

	// Waits for the trigger to become ready and then executes the trigger.
	virtual bool waitAndTrigger(std::chrono::milliseconds timeout) noexcept = 0;

	// Acquire an image.
	// WARNING: acquisition must be started manually, however,
	// acquisition can be stopped automatically, eg. when a certain
	// number of images has been acquired.
	virtual bool acquire(std::chrono::milliseconds timeout) noexcept = 0;

	// Start image acquisition and stop after nImages have been acquired.
	// If nImages is 0, the camera will keep acquiring indefinitely.
	virtual bool startAcquisition(size_t nImages) noexcept = 0;

	// Stop image acquisition.
	virtual void stopAcquisition() noexcept = 0;

	// Return acquisition state.
	//
	// BUG: this checks the pylon grab state, but not the (camera)
	// acquisition ('AcquisitionStart/Stop') state, i.e. we could be
	// grabbing but not acquiring :D
	// This happens, for example, when using the 'SingleFrame'
	// 'AcquisitionMode': the camera will have executed 'AcquisitionStop'
	// internally, but pylon will report: IsGrabbing() == true.
	// Hence we should avoid using the 'SingleFrame' acquisition mode.
	[[nodiscard]] virtual bool isAcquiring() const noexcept = 0;

	// Check if the camera is initialized (device attached and open).
	[[nodiscard]] virtual bool isInitialized() const noexcept = 0;

	// Check if the camera device is attached.
	[[nodiscard]] virtual bool isAttached() const noexcept = 0;

	// Get the acquired result as a raw image.
	//
	// NOTE: this does not transfer ownership of the underlying
	// acquisition result.
	// The caller should copy the returned buffer if data persistence
	// is required.
	[[nodiscard]] virtual std::optional<Image> getImage() noexcept = 0;

	// Execute a GenICam command on the camera device.
	// Returns false if there was an error.
	//
	// NOTE: does not check whether the command was executed or
	// whether execution was successful.
	virtual bool cmdExecute(const char* cmd) noexcept = 0;

	// Report if command execution finished.
	[[nodiscard]] virtual bool cmdIsDone(const char* cmd) noexcept = 0;
};

}  // namespace pylonshim
}  // namespace beholder

#endif	// BEHOLDER_PYLONSHIM_CAMERA_H
