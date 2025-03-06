// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BEHOLDER_CAMERA_PYLON_CAMERA_H
#define BEHOLDER_CAMERA_PYLON_CAMERA_H

#include <cstddef>
#include <optional>

#include "beholder/camera/Camera.h"
#include "beholder/camera/Parameter.h"
#include "beholder/capi/Image.h"
#include "beholder/embed/Loader.h"

class PylonShimCameraInterface;

namespace beholder {
namespace camera {
namespace pylon {

// PylonCamera represents a physical camera device.
class PylonCamera : public ::beholder::camera::CameraInterface {
private:
	using CameraHandle = embed::Loader::ClassHandle<PylonShimCameraInterface>;

	CameraHandle handle_;  // the underlying implementation symbol handle

public:
	// Default constructor.
	// The camera must be initialized with PylonCamera::init before use.
	//
	// TODO: implement
	PylonCamera();

	PylonCamera(const PylonCamera&) = delete;
	PylonCamera(PylonCamera&&) = delete;
	PylonCamera& operator=(const PylonCamera&) = delete;
	PylonCamera& operator=(PylonCamera&&) = delete;

	// Default destructor.
	// Defined in the source because unique_ptr complains about
	// incomplete types.
	~PylonCamera() override;

	// Initialize camera device.
	// The device is attached and open after initialization.
	//
	// NOTE: takes ownership of the supplied device.
	//
	// TODO: we could initialize it with/from a TransportLayer, so that
	// we don't have to expose pylon stuff at all.
	bool init(void* device) noexcept override;

	// Get camera parameters
	ParamVector getParams(Parameter::AccessMode mode) noexcept override;

	// Set camera parameters in the order provided.
	// Returns true if no errors ocurred.
	bool setParams(CParamSpan params) noexcept override;

	// Execute a trigger.
	bool trigger(TriggerType typ) noexcept override;

	// Waits for the trigger to become ready and then executes the trigger.
	bool
	waitAndTrigger(Milliseconds timeout, TriggerType typ) noexcept override;

	// Acquire an image.
	// WARNING: acquisition must be started manually, however,
	// acquisition can be stopped automatically, eg. when a certain
	// number of images has been acquired.
	bool acquire(Milliseconds timeout) override;

	// Start image acquisition and stop after nImages have been acquired.
	// If nImages is 0, the camera will keep acquiring indefinitely.
	bool startAcquisition(size_t nImages) noexcept override;

	// Stop image acquisition.
	void stopAcquisition() noexcept override;

	// Return acquisition state.
	//
	// BUG: this checks the pylon grab state, but not the (camera)
	// acquisition ('AcquisitionStart/Stop') state, i.e. we could be
	// grabbing but not acquiring :D
	// This happens, for example, when using the 'SingleFrame'
	// 'AcquisitionMode': the camera will have executed 'AcquisitionStop'
	// internally, but pylon will report: IsGrabbing() == true.
	// Hence we should avoid using the 'SingleFrame' acquisition mode.
	[[nodiscard]] bool isAcquiring() const noexcept override;

	// Check if the camera device is attached.
	[[nodiscard]] bool isAttached() const noexcept override;

	// Check if the camera is initialized (device attached and open).
	[[nodiscard]] bool isInitialized() const noexcept override;

	// Get the acquired result as a raw image.
	//
	// NOTE: this does not transfer ownership of the underlying
	// acquisition result.
	// The receiver should copy the returned buffer if data persistence
	// is required.
	std::optional<Image> getImage() noexcept override;

	// Execute a GenICam command on the camera device.
	// Returns false if there was an error.
	//
	// WARNING: does not check whether the command was executed or
	// whether execution was successful.
	bool cmdExecute(const char* cmd) noexcept override;

	// Report if command execution finished.
	bool cmdIsDone(const char* cmd) noexcept override;
};

}  // namespace pylon
}  // namespace camera
}  // namespace beholder

#endif	// BEHOLDER_CAMERA_PYLON_CAMERA_H
