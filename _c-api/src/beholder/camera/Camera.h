// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

// Camera device class definitions.

#ifndef BEHOLDER_CAMERA_CAMERA_H
#define BEHOLDER_CAMERA_CAMERA_H

#include <chrono>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>

#include "beholder/camera/Backends.h"
#include "beholder/camera/Parameter.h"
#include "beholder/capi/Image.h"

namespace beholder {
namespace camera {

class CameraInterface;
using CamPtr = std::unique_ptr<CameraInterface>;
using Milliseconds = std::chrono::milliseconds;

// The default timeout for acquiring an image.
static constexpr Milliseconds DfltAcqTimeout{1000};

// The default trigger timeout.
static constexpr Milliseconds DfltTriggerTimeout{100};

// Supported camera acquisition trigger types.
enum class TriggerType { Unknown = -1, Software };

// CameraInterface is a base class for the underlying camera implementations.
class CameraInterface {
private:
	CameraInterface() = default;

public:
	CameraInterface(const CameraInterface&) = delete;
	CameraInterface(CameraInterface&&) = delete;
	CameraInterface& operator=(const CameraInterface&) = delete;
	CameraInterface& operator=(CameraInterface&&) = delete;
	virtual ~CameraInterface() = default;

	[[nodiscard]] virtual bool init(void* device) noexcept = 0;

	virtual ParamVector getParams(Parameter::AccessMode mode) noexcept = 0;
	virtual bool setParams(CParamSpan params) noexcept = 0;

	virtual bool trigger(TriggerType typ) noexcept = 0;
	virtual bool
	waitAndTrigger(Milliseconds timeout, TriggerType typ) noexcept = 0;

	virtual bool acquire(Milliseconds timeout) = 0;
	[[nodiscard]] virtual bool startAcquisition(size_t nImages) noexcept = 0;
	virtual void stopAcquisition() noexcept = 0;
	[[nodiscard]] virtual bool isAcquiring() const noexcept = 0;
	[[nodiscard]] virtual bool isAttached() const noexcept = 0;
	[[nodiscard]] virtual bool isInitialized() const noexcept = 0;

	virtual std::optional<Image> getImage() noexcept = 0;

	virtual bool cmdExecute(const char* cmd) noexcept = 0;
	[[nodiscard]] virtual bool cmdIsDone(const char* cmd) noexcept = 0;
};

// Camera represents a physical camera device.
class Camera {
private:
	// The underlying implementation.
	CamPtr impl_;

public:
	// Default constructor.
	// The camera must be initialized with Camera::init before use.
	explicit Camera(Backend b = Backend::Pylon) : impl_{createCamera(b)} {}

	// Acquire an image.
	// WARNING: acquisition must be started manually, however,
	// acquisition can be stopped automatically, eg. when a certain
	// number of images has been acquired.
	bool acquire(Milliseconds timeout = DfltAcqTimeout) {
		return impl_->acquire(timeout);
	}

	// Execute a GenICam command on the camera device.
	// Returns false if there was an error.
	//
	// WARNING: does not check whether the command was executed or
	// whether execution was successful.
	bool cmdExecute(const std::string& cmd) noexcept {
		return impl_->cmdExecute(cmd.c_str());
	}

	// Execute a GenICam command on the camera device.
	// Returns false if there was an error.
	//
	// WARNING: does not check whether the command was executed or
	// whether execution was successful.
	bool cmdExecute(const char* cmd) noexcept { return impl_->cmdExecute(cmd); }

	// Report if command execution finished.
	[[nodiscard]] bool cmdIsDone(const std::string& cmd) noexcept {
		return impl_->cmdIsDone(cmd.c_str());
	}

	// Report if command execution finished.
	[[nodiscard]] bool cmdIsDone(const char* cmd) noexcept {
		return impl_->cmdIsDone(cmd);
	}

	// Get the acquired result as a raw image.
	//
	// NOTE: this does not transfer ownership of the underlying
	// acquisition result.
	// The receiver should copy the returned buffer if data persistence
	// is required.
	std::optional<Image> getImage() noexcept { return impl_->getImage(); }

	// Get camera parameters
	ParamVector getParams(Parameter::AccessMode mode =
							  Parameter::AccessMode::ReadWrite) noexcept {
		return impl_->getParams(mode);
	}

	// Initialize camera device.
	// The device is attached and open after initialization.
	//
	// NOTE: takes ownership of the supplied device.
	//
	// TODO: we could initialize it with/from a TransportLayer, so that
	// we don't have to expose pylon stuff at all.
	// TODO: change from void*
	[[nodiscard]] bool init(void* device) noexcept {
		return impl_->init(device);
	}

	// Return acquisition state.
	//
	// BUG: this checks the pylon grab state, but not the (camera)
	// acquisition ('AcquisitionStart/Stop') state, i.e. we could be
	// grabbing but not acquiring :D
	// This happens, for example, when using the 'SingleFrame'
	// 'AcquisitionMode': the camera will have executed 'AcquisitionStop'
	// internally, but pylon will report: IsGrabbing() == true.
	// Hence we should avoid using the 'SingleFrame' acquisition mode.
	[[nodiscard]] bool isAcquiring() const noexcept {
		return impl_->isAcquiring();
	}

	// Check if the camera is initialized (device attached and open).
	[[nodiscard]] bool isInitialized() const noexcept {
		return impl_->isInitialized();
	}

	// Check if the camera device is attached.
	[[nodiscard]] bool isAttached() const noexcept {
		return impl_->isAttached();
	}

	// Set camera parameters in the order provided.
	// Returns true if no errors ocurred.
	bool setParams(CParamSpan params) noexcept {
		return impl_->setParams(params);
	}

	// Start image acquisition and stop after nImages have been acquired.
	// If nImages is 0, the camera will keep acquiring indefinitely.
	[[nodiscard]] bool startAcquisition(size_t nImages = 0UL) noexcept {
		return impl_->startAcquisition(nImages);
	}

	// Stop image acquisition.
	void stopAcquisition() noexcept { impl_->stopAcquisition(); }

	// Execute a trigger.
	bool trigger(TriggerType typ = TriggerType::Software) noexcept {
		return impl_->trigger(typ);
	}

	// Waits for the trigger to become ready and then executes the trigger.
	bool waitAndTrigger(Milliseconds timeout = DfltTriggerTimeout,
						TriggerType typ = TriggerType::Software) noexcept {
		return impl_->waitAndTrigger(timeout, typ);
	}
};

}  // namespace camera
}  // namespace beholder

#endif	// BEHOLDER_CAMERA_CAMERA_H
