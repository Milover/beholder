// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

#include "beholder/camera/Camera.h"

#include <GenApi/INode.h>
#include <pylon/Device.h>
#include <pylon/ECleanup.h>
#include <pylon/ERegistrationMode.h>
#include <pylon/ETimeoutHandling.h>
#include <pylon/GrabResultPtr.h>
#include <pylon/InstantCamera.h>
#include <pylon/Parameter.h>
#include <pylon/PixelType.h>

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <optional>
#include <utility>

#include "beholder/camera/Exception.h"
#include "beholder/camera/ParamEntry.h"
#include "beholder/camera/internal/DefaultConfigurator.h"
#include "beholder/camera/internal/GenAPIUtils.h"
#include "beholder/capi/Image.h"
#include "beholder/util/Enums.h"

namespace beholder {

void Camera::Deleter::operator()(Pylon::CInstantCamera* cam) noexcept {
	if (static_cast<bool>(cam)) {
		cam->DestroyDevice();
		delete cam;
	}
}

bool Camera::triggerImpl(TriggerType typ) {
	switch (typ) {
		case TriggerType::Software: {
			cam_->ExecuteSoftwareTrigger();
			return true;
			break;
		}
		case TriggerType::Unknown: {
			return false;
			break;
		}
	}
	return false;
}

Camera::Camera()
	: cam_{new Pylon::CInstantCamera{}, Deleter{}},
	  res_{new Pylon::CGrabResultPtr{}} {
	cam_->RegisterConfiguration(new internal::DefaultConfigurator,
								Pylon::RegistrationMode_ReplaceAll,
								Pylon::Cleanup_Delete);
}

bool Camera::acquire(std::chrono::milliseconds timeout) {
	if (!isAttached()) {
		throw Exception{"no camera device attached"};
	}
	if (!isAcquiring()) {
		throw Exception{"acquisition not started"};
	}
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
	return false;
}

bool Camera::cmdExecute(const std::string& cmd) noexcept {
	return cmdExecute(cmd.c_str());
}

bool Camera::cmdExecute(const char* cmd) noexcept {
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

bool Camera::cmdIsDone(const std::string& cmd) noexcept {
	return cmdIsDone(cmd.c_str());
}

bool Camera::cmdIsDone(const char* cmd) noexcept {
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

std::optional<Image> Camera::getImage() noexcept {
	// not sure if this can throw, so we're being careful
	try {
		auto& res{*res_};
		if (!res.IsValid()) {
			return std::nullopt;
		}
		std::size_t step{0UL};
		return std::optional{Image{
			static_cast<std::size_t>(res->GetID()),
			static_cast<int>(res->GetHeight()),
			static_cast<int>(res->GetWidth()),
			static_cast<std::int64_t>(res->GetPixelType()), res->GetBuffer(),
			res->GetStride(step) ? step : 0UL,
			static_cast<std::size_t>(Pylon::BitPerPixel(res->GetPixelType()))}};
	} catch (const Pylon::GenericException& e) {
		std::cerr << "could get raw image data: " << e.what() << std::endl;
	} catch (...) {
		std::cerr << "could get raw image data" << std::endl;
	}
	return std::nullopt;
}

ParamList Camera::getParams(ParamAccessMode mode) {
	ParamList params;
	bool (*condition)(GenApi::INode*){nullptr};
	switch (mode) {
		case ParamAccessMode::Read: {
			condition = +[](GenApi::INode* n) -> bool {
				return GenApi::IsAvailable(n) && n->IsFeature() &&
					   GenApi::IsReadable(n);
			};
			break;
		}
		case ParamAccessMode::ReadWrite: {
			condition = +[](GenApi::INode* n) -> bool {
				return GenApi::IsAvailable(n) && n->IsFeature() &&
					   GenApi::IsReadable(n) && GenApi::IsWritable(n);
			};
			break;
		}
		case ParamAccessMode::Unknown: {
			condition = +[](GenApi::INode*) -> bool { return false; };
			break;
		}
	}
	auto nodes{internal::convert<GenApi::NodeList_t, GenApi::INode*>(
		condition, cam_->GetNodeMap(), cam_->GetTLNodeMap(),
		cam_->GetStreamGrabberNodeMap(), cam_->GetEventGrabberNodeMap(),
		cam_->GetInstantCameraNodeMap())};

	params.reserve(nodes.size());
	for (auto&& n : nodes) {
		const auto typ{enums::from<ParamType>(n->GetPrincipalInterfaceType())};
		if (typ == ParamType::Unknown) {
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

bool Camera::isAcquiring() const noexcept { return cam_->IsGrabbing(); }

bool Camera::init(Pylon::IPylonDevice* d) noexcept {
	try {
		cam_->Attach(d, Pylon::Cleanup_Delete);
		cam_->Open();
		return true;
	} catch (const Pylon::GenericException& e) {
		std::cerr << "could not initialize camera: " << e.what() << std::endl;
	} catch (const Exception& e) {
		std::cerr << "could not initialize camera: " << e.what() << std::endl;
	} catch (...) {
		std::cerr << "could not initialize camera" << std::endl;
	}
	return false;
}

bool Camera::isInitialized() const noexcept {
	return isAttached() && cam_->IsOpen();
}

bool Camera::isAttached() const noexcept {
	return cam_->IsPylonDeviceAttached() && !cam_->IsCameraDeviceRemoved();
}

bool Camera::setParams(const ParamList& params) noexcept {
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
			std::cerr << "could not set \"" << p.name << ": " << e.what()
					  << std::endl;
		} catch (...) {
			ok = false;
			std::cerr << "could not set \"" << p.name << "\"" << std::endl;
		}
	}
	return ok;
}

bool Camera::startAcquisition(std::size_t nImages) noexcept {
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

void Camera::stopAcquisition() noexcept { cam_->StopGrabbing(); }

bool Camera::trigger(TriggerType typ) noexcept {
	try {
		return triggerImpl(typ);
	} catch (const Pylon::GenericException& e) {
		std::cerr << "could not execute trigger: " << e.what() << std::endl;
	} catch (const Exception& e) {
		std::cerr << "could not execute trigger: " << e.what() << std::endl;
	} catch (...) {
		std::cerr << "could not execute trigger: " << std::endl;
	}
	return false;
}

bool Camera::waitAndTrigger(std::chrono::milliseconds timeout,
							TriggerType typ) noexcept {
	try {
		if (!cam_->CanWaitForFrameTriggerReady()) {
			std::cerr << "could not execute trigger: "
					  << "camera device cannot wait for trigger" << std::endl;
		}
		if (cam_->WaitForFrameTriggerReady(timeout.count(),
										   Pylon::TimeoutHandling_Return)) {
			return triggerImpl(typ);
		}
	} catch (const Pylon::GenericException& e) {
		std::cerr << "could not execute trigger: " << e.what() << std::endl;
	} catch (const Exception& e) {
		std::cerr << "could not execute trigger: " << e.what() << std::endl;
	} catch (...) {
		std::cerr << "could not execute trigger: " << std::endl;
	}
	return false;
}

}  // namespace beholder
