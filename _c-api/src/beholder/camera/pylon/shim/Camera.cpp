// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include "beholder/camera/pylon/shim/Camera.h"

#include <GenApi/INode.h>
#include <pylon/Device.h>
#include <pylon/ECleanup.h>
#include <pylon/ERegistrationMode.h>
#include <pylon/ETimeoutHandling.h>
#include <pylon/GrabResultPtr.h>
#include <pylon/InstantCamera.h>
#include <pylon/Parameter.h>
#include <pylon/PixelType.h>
#include <pylon/TypeMappings.h>

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "beholder/camera/pylon/shim/Configurator.h"
#include "beholder/camera/pylon/shim/Convert.h"
#include "beholder/camera/pylon/shim/Export.h"
#include "beholder/capi/Image.h"
#include "beholder/util/Enums.h"

namespace beholder {
namespace pylon {
namespace shim {

class BH_PYLON_SHIM_API CameraImpl : public Camera {
private:
	struct Deleter {
		void operator()(Pylon::CInstantCamera* cam) noexcept {
			if (static_cast<bool>(cam)) {
				cam->DestroyDevice();
				delete cam;
			}
		}
	};
	using CamPtr = std::unique_ptr<Pylon::CInstantCamera, Deleter>;
	using ResPtr = std::unique_ptr<Pylon::CGrabResultPtr>;

	CamPtr cam_;  // underlying camera device
	ResPtr res_;  // underlying camera acquisition result resource

public:
	CameraImpl() noexcept;

	CameraImpl(const CameraImpl&) = delete;
	CameraImpl(CameraImpl&&) = default;
	CameraImpl& operator=(const CameraImpl&) = delete;
	CameraImpl& operator=(CameraImpl&&) = default;

	~CameraImpl() override = default;

	bool init(Pylon::IPylonDevice* d) noexcept override;

	ParamVector getParams(Parameter::AccessMode mode) override;
	bool setParams(CParamSpan params) noexcept override;

	bool trigger() noexcept override;
	bool waitAndTrigger(std::chrono::milliseconds timeout) noexcept override;

	bool acquire(std::chrono::milliseconds timeout) noexcept override;
	bool startAcquisition(size_t nImages) noexcept override;
	void stopAcquisition() noexcept override;

	[[nodiscard]] bool isAcquiring() const noexcept override;
	[[nodiscard]] bool isInitialized() const noexcept override;
	[[nodiscard]] bool isAttached() const noexcept override;

	std::optional<Image> getImage() noexcept override;

	bool cmdExecute(const char* cmd) noexcept override;
	bool cmdIsDone(const char* cmd) noexcept override;
};

CameraImpl::CameraImpl() noexcept {
	try {
		cam_.reset(new Pylon::CInstantCamera{});
		res_.reset(new Pylon::CGrabResultPtr{});  // NOLINT(*make-unique)
	} catch (...) {
		std::cerr << "could not construct camera" << std::endl;
		std::exit(EXIT_FAILURE);
	}
	try {
		cam_->RegisterConfiguration(new Configurator,
									Pylon::RegistrationMode_ReplaceAll,
									Pylon::Cleanup_Delete);
	} catch (const Pylon::GenericException& e) {
		std::cerr << "error during camera configuration: " << e.what()
				  << std::endl;
	} catch (...) {
		std::cerr << "error during camera configuration: " << std::endl;
	}
}

bool CameraImpl::acquire(std::chrono::milliseconds timeout) noexcept {
	if (!isAttached()) {
		std::cerr << "no camera device attached" << std::endl;
		return false;
	}
	if (!isAcquiring()) {
		std::cerr << "acquisition not started" << std::endl;
		return false;
	}
	try {
		auto& res{*res_};
		const bool success{cam_->RetrieveResult(timeout.count(), res,
												Pylon::TimeoutHandling_Return)};
		if (success && res->GrabSucceeded()) {
			if (res->HasCRC() && !res->CheckCRC()) {
				std::cerr << "CRC check failed" << std::endl;
			} else {
				return true;
			}
		} else if (success) {
			std::cerr << "error code: " << res->GetErrorCode() << '\t'
					  << res->GetErrorDescription() << std::endl;
		} else {
			std::cerr << "acquisition timed out" << std::endl;
		}
	} catch (const Pylon::GenericException& e) {
		std::cerr << "could not acquire image: " << e.what() << std::endl;
	} catch (...) {
		std::cerr << "could not acquire image" << std::endl;
	}
	return false;
}

bool CameraImpl::cmdExecute(const char* cmd) noexcept {
	try {
		Pylon::CCommandParameter(cam_->GetNodeMap(), cmd).Execute();
		return true;
	} catch (const Pylon::GenericException& e) {
		std::cerr << "could not execute command: " << e.what() << std::endl;
	} catch (...) {
		std::cerr << "could not execute command" << std::endl;
	}
	return false;
}

bool CameraImpl::cmdIsDone(const char* cmd) noexcept {
	try {
		return Pylon::CCommandParameter(cam_->GetNodeMap(), cmd).IsDone();
	} catch (const Pylon::GenericException& e) {
		std::cerr << "could not check command execution status: " << e.what()
				  << std::endl;
	} catch (...) {
		std::cerr << "could not check command execution status: " << std::endl;
	}
	return false;
}

std::optional<Image> CameraImpl::getImage() noexcept {
	// not sure if this can throw, so we're being careful
	try {
		auto& res{*res_};
		if (!res.IsValid()) {
			return std::nullopt;
		}
		size_t step{0UL};
		return std::optional{Image{
			static_cast<size_t>(res->GetID()),
			static_cast<int>(res->GetHeight()),
			static_cast<int>(res->GetWidth()),
			static_cast<int64_t>(res->GetPixelType()), res->GetBuffer(),
			res->GetStride(step) ? step : 0UL,
			static_cast<size_t>(Pylon::BitPerPixel(res->GetPixelType()))}};
	} catch (const Pylon::GenericException& e) {
		std::cerr << "could not get raw image data: " << e.what() << std::endl;
	} catch (...) {
		std::cerr << "could not get raw image data" << std::endl;
	}
	return std::nullopt;
}

ParamVector CameraImpl::getParams(Parameter::AccessMode mode) {
	ParamVector params;
	bool (*condition)(GenApi::INode*){nullptr};
	switch (mode) {
		case Parameter::AccessMode::Read: {
			condition = +[](GenApi::INode* n) -> bool {
				return GenApi::IsAvailable(n) && n->IsFeature() &&
					   GenApi::IsReadable(n);
			};
			break;
		}
		case Parameter::AccessMode::ReadWrite: {
			condition = +[](GenApi::INode* n) -> bool {
				return GenApi::IsAvailable(n) && n->IsFeature() &&
					   GenApi::IsReadable(n) && GenApi::IsWritable(n);
			};
			break;
		}
		case Parameter::AccessMode::Unknown: {
			condition = +[](GenApi::INode*) -> bool { return false; };
			break;
		}
	}
	auto nodes{convert<GenApi::NodeList_t, GenApi::INode*>(
		condition, cam_->GetNodeMap(), cam_->GetTLNodeMap(),
		cam_->GetStreamGrabberNodeMap(), cam_->GetEventGrabberNodeMap(),
		cam_->GetInstantCameraNodeMap())};

	params.reserve(nodes.size());
	for (auto&& n : nodes) {
		const auto typ{
			enums::from<Parameter::Type>(n->GetPrincipalInterfaceType())};
		if (typ == Parameter::Type::Unknown) {
			continue;
		}
		Pylon::CParameter par{std::forward<GenApi::INode*>(n)};
		params.emplace_back(par.GetInfo(Pylon::ParameterInfo_Name).c_str(),
							par.ToString().c_str(), typ);
	}
	params.shrink_to_fit();
	std::sort(
		params.begin(), params.end(),
		[](const auto& a, const auto& b) -> bool { return a.name < b.name; });
	return params;
}

bool CameraImpl::isAcquiring() const noexcept { return cam_->IsGrabbing(); }

bool CameraImpl::init(Pylon::IPylonDevice* d) noexcept {
	if (!static_cast<bool>(d)) {
		std::cerr << "could not initialize camera: bad device" << std::endl;
		return false;
	}
	try {
		cam_->Attach(d, Pylon::Cleanup_Delete);
		cam_->Open();
		return true;
	} catch (const Pylon::GenericException& e) {
		std::cerr << "could not initialize camera: " << e.what() << std::endl;
	} catch (...) {
		std::cerr << "could not initialize camera" << std::endl;
	}
	return false;
}

bool CameraImpl::isInitialized() const noexcept {
	return isAttached() && cam_->IsOpen();
}

bool CameraImpl::isAttached() const noexcept {
	return cam_->IsPylonDeviceAttached() && !cam_->IsCameraDeviceRemoved();
}

bool CameraImpl::setParams(CParamSpan params) noexcept {
	if (!isInitialized()) {
		std::cerr << "could not set parameters, camera uninitialized"
				  << std::endl;
		return false;
	}
	bool ok{true};
	for (const auto& p : params) {
		try {
			Pylon::CParameter par{cam_->GetNodeMap(), p.name.c_str()};
			par.FromString(p.value.c_str());
		} catch (const Pylon::GenericException& e) {
			ok = false;
			std::cerr << "could not set \"" << p.name << "\": " << e.what()
					  << std::endl;
		} catch (...) {
			ok = false;
			std::cerr << "could not set \"" << p.name << "\"" << std::endl;
		}
	}
	return ok;
}

bool CameraImpl::startAcquisition(size_t nImages) noexcept {
	// XXX: not sure what happens here if the camera gets disconnected
	if (isAcquiring()) {
		return true;
	}
	try {
		if (nImages == 0) {
			cam_->StartGrabbing();
		} else {
			cam_->StartGrabbing(nImages);
		}
		return true;
	} catch (const Pylon::GenericException& e) {
		std::cerr << "could not start acquisition: " << e.what() << std::endl;
	} catch (...) {
		std::cerr << "could not start acquisition" << std::endl;
	}
	return false;
}

void CameraImpl::stopAcquisition() noexcept { cam_->StopGrabbing(); }

bool CameraImpl::trigger() noexcept {
	try {
		cam_->ExecuteSoftwareTrigger();
		return true;
	} catch (const Pylon::GenericException& e) {
		std::cerr << "could not execute trigger: " << e.what() << std::endl;
	} catch (...) {
		std::cerr << "could not execute trigger: " << std::endl;
	}
	return false;
}

bool CameraImpl::waitAndTrigger(std::chrono::milliseconds timeout) noexcept {
	try {
		if (!cam_->CanWaitForFrameTriggerReady()) {
			std::cerr << "could not execute trigger: "
					  << "camera device cannot wait for trigger" << std::endl;
		}
		if (cam_->WaitForFrameTriggerReady(timeout.count(),
										   Pylon::TimeoutHandling_Return)) {
			cam_->ExecuteSoftwareTrigger();
			return true;
		}
	} catch (const Pylon::GenericException& e) {
		std::cerr << "could not execute trigger: " << e.what() << std::endl;
	} catch (...) {
		std::cerr << "could not execute trigger: " << std::endl;
	}
	return false;
}

}  // namespace shim
}  // namespace pylon
}  // namespace beholder

extern "C" {

BH_PYLON_SHIM_API beholder::pylon::shim::Camera* pylonCamera_create() {
	return new beholder::pylon::shim::CameraImpl{};
}

BH_PYLON_SHIM_API void pylonCamera_delete(beholder::pylon::shim::Camera* p) {
	delete p;
}

}  // extern "C"
