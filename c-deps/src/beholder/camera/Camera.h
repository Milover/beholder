// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

// Camera device class definitions.

#ifndef BEHOLDER_CAMERA_CAMERA_H
#define BEHOLDER_CAMERA_CAMERA_H

#include <algorithm>
#include <chrono>
#include <iostream>
#include <memory>
#include <optional>
#include <ratio>

#include "beholder/BeholderExport.h"
#include "beholder/camera/Exception.h"
#include "beholder/camera/ParamEntry.h"
#include "beholder/capi/Image.h"

namespace Pylon {
class CGrabResultPtr;
class CInstantCamera;
class IPylonDevice;
}  // namespace Pylon

namespace beholder {

// Supported camera acquisition trigger types.
enum class BH_API TriggerType { Software, Unknown = -1 };

// The default timeout for acquiring an image.
inline static constexpr std::chrono::milliseconds DfltAcqTimeout{1000};

// The default trigger timeout.
inline static constexpr std::chrono::milliseconds DfltTriggerTimeout{100};

// Camera represents a physical camera device.
class BH_API Camera {
private:
	// Deleter is a helper class for releasing the underlying camera device
	// resources.
	struct Deleter {
		void operator()(Pylon::CInstantCamera* cam) noexcept;
	};

	// The underlying camera device.
	std::unique_ptr<Pylon::CInstantCamera, Deleter> cam_;
	// Underlying camera acquisition result.
	std::unique_ptr<Pylon::CGrabResultPtr> res_;

protected:
	// Execute a trigger.
	bool triggerImpl(TriggerType typ);

public:
	// Default constructor.
	// The camera must be initialized with Camera::init before use.
	Camera();

	Camera(const Camera&) = delete;
	Camera(Camera&&) = delete;

	// Default destructor.
	// Defined in the source because unique_ptr complains about
	// incomplete types.
	~Camera();

	Camera& operator=(const Camera&) = delete;
	Camera& operator=(Camera&&) = delete;

	//- Acquire an image.
	//	WARNING: acquisition must be started manually, however,
	//	acquisition can be stopped automatically, eg. when a certain
	//	number of images has been acquired.
	bool acquire(std::chrono::milliseconds timeout = DfltAcqTimeout);

	// Execute a GenICam command on the camera device.
	// Returns false if there was an error.
	//
	// WARNING: does not check whether the command was executed or
	// whether execution was successful.
	bool cmdExecute(const std::string& cmd) noexcept;

	// Execute a GenICam command on the camera device.
	// Returns false if there was an error.
	//
	// WARNING: does not check whether the command was executed or
	// whether execution was successful.
	bool cmdExecute(const char* cmd) noexcept;

	// Report if command execution finished.
	bool cmdIsDone(const std::string& cmd) noexcept;

	// Report if command execution finished.
	bool cmdIsDone(const char* cmd) noexcept;

	// Get the acquired result as a raw image.
	//
	// NOTE: this does not transfer ownership of the underlying
	// acquisition result.
	// The receiver should copy the returned buffer if data persistence
	// is required.
	std::optional<Image> getImage() noexcept;

#ifndef NDEBUG
	// Get reference to the underlying pylon camera.
	Pylon::CInstantCamera* getPtr() const noexcept { return cam_.get(); }

	// Get reference to the acquired result.
	Pylon::CGrabResultPtr* getResultPtr() const noexcept { return res_.get(); }
#endif

	// Get camera parameters
	ParamList getParams(ParamAccessMode mode = ParamAccessMode::ReadWrite);

	// Initialize camera device.
	// The device is attached and open after initialization.
	//
	// NOTE: takes ownership of the supplied device.
	//
	// TODO: we could initialize it with/from a TransportLayer, so that
	// we don't have to expose pylon stuff at all.
	bool init(Pylon::IPylonDevice* d) noexcept;

	// Return acquisition state.
	//
	// BUG: this checks the pylon grab state, but not the (camera)
	// acquisition ('AcquisitionStart/Stop') state, i.e. we could be
	// grabbing but not acquiring :D
	// This happens, for example, when using the 'SingleFrame'
	// 'AcquisitionMode': the camera will have executed 'AcquisitionStop'
	// internally, but pylon will report: IsGrabbing() == true.
	// Hence we should avoid using the 'SingleFrame' acquisition mode.
	[[nodiscard]] bool isAcquiring() const noexcept;

	// Check if the camera is initialized (device attached and open).
	[[nodiscard]] bool isInitialized() const noexcept;

	// Check if the camera device is attached.
	[[nodiscard]] bool isAttached() const noexcept;

	// Set camera parameters in the order provided.
	// Returns true if no errors ocurred.
	bool setParams(const ParamList& params) noexcept;

	// Start image acquisition and stop after nImages have been acquired.
	// If nImages is 0, the camera will keep acquiring indefinitely.
	bool startAcquisition(std::size_t nImages = 0UL) noexcept;

	// Stop image acquisition.
	void stopAcquisition() noexcept;

	// Execute a trigger.
	bool trigger(TriggerType typ = TriggerType::Software) noexcept;

	// Waits for the trigger to become ready and then executes the trigger.
	bool waitAndTrigger(std::chrono::milliseconds timeout = DfltTriggerTimeout,
						TriggerType typ = TriggerType::Software) noexcept;
};

}  // namespace beholder

#endif	// BEHOLDER_CAMERA_CAMERA_H
