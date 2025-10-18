// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BEHOLDER_CAMERA_PYLON_CAMERA_H
#define BEHOLDER_CAMERA_PYLON_CAMERA_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>

#include "beholder/camera/Backends.h"
#include "beholder/camera/CameraInterface.h"
#include "beholder/camera/DeviceClass.h"
#include "beholder/camera/Parameter.h"
#include "beholder/camera/TriggerType.h"
#include "beholder/capi/Image.h"
#include "beholder/embed/Loader.h"
#include "beholder/embed/Unpacker.h"
#include "beholder/util/Enums.h"

namespace beholder {
namespace pylonshim {
class API;
class Camera;
class TransportLayer;
}  // namespace pylonshim
}  // namespace beholder

namespace beholder {
namespace camera {
namespace pylon {

using CameraInterface = ::beholder::camera::detail::CameraImpl::Type;
using CamPtr = ::beholder::camera::detail::CameraImpl::Ptr;
using Milliseconds = CameraInterface::Milliseconds;

// Camera factory.
[[nodiscard]] CamPtr createCamera(DeviceClass dc);

// Camera represents a physical camera device.
class Camera : public CameraInterface {
private:
	struct Runtime;

	using APIHandle = embed::Loader::ClassHandle<pylonshim::API>;
	using CamHandle = embed::Loader::ClassHandle<pylonshim::Camera>;
	using TLHandle = embed::Loader::ClassHandle<pylonshim::TransportLayer>;
	using TLArray = std::array<TLHandle, enums::to(DeviceClass::_max)>;
	using RTHandle = std::unique_ptr<Runtime>;

	struct Runtime {
		embed::Unpacker upk;  // shared object archive unpacker
		embed::Loader ldr;	  // shared object loader
		APIHandle api;		  // pylon runtime
		TLArray tls;		  // transport layers indexed by device class

		std::uint32_t count{};	// FIXME: needs to be atomic

		Runtime() noexcept;
	};

	inline static RTHandle runtime_{};	// global runtime
	CamHandle cam_;	  // underlying camera implementation symbol handle
	DeviceClass dc_;  // camera transport layer type

public:
	// Default constructor.
	// The camera must be initialized with Camera::init before use.
	explicit Camera(DeviceClass dc);

	Camera(const Camera&) = delete;
	Camera(Camera&&) = delete;
	Camera& operator=(const Camera&) = delete;
	Camera& operator=(Camera&&) = delete;

	// Default destructor.
	//
	// Defined in the source because unique_ptr complains about
	// incomplete types.
	~Camera() override;

	// The device is attached and open after initialization.
	[[nodiscard]] bool init(const char* designator, Milliseconds timeout,
							bool reboot) noexcept override;

	// Get camera parameters
	[[nodiscard]] ParamVector
	getParams(Parameter::AccessMode mode) noexcept override;

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
	bool acquire(Milliseconds timeout) noexcept override;

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
	[[nodiscard]] std::optional<Image> getImage() noexcept override;

	// Execute a GenICam command on the camera device.
	// Returns false if there was an error.
	//
	// WARNING: does not check whether the command was executed or
	// whether execution was successful.
	bool cmdExecute(const char* cmd) noexcept override;

	// Report if command execution finished.
	[[nodiscard]] bool cmdIsDone(const char* cmd) noexcept override;
};

}  // namespace pylon
}  // namespace camera
}  // namespace beholder

#endif	// BEHOLDER_CAMERA_PYLON_CAMERA_H
