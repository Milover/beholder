// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include "beholder/camera/Camera.h"

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "beholder/camera/Backends.h"
#include "beholder/camera/CameraInterface.h"
#include "beholder/camera/Parameter.h"
#include "beholder/camera/TriggerType.h"
#include "beholder/capi/Image.h"

namespace beholder {
namespace camera {

Camera::Camera(Config cfg)
	: cfg_{std::move(cfg)},
	  impl_{createCamera(cfg_.deviceClass, cfg_.backend)} {}

ParamVector Camera::getParams(Parameter::AccessMode mode) noexcept {
	return impl_->getParams(mode);
}
bool Camera::setParams(CParamSpan params) noexcept {
	return impl_->setParams(params);
}

bool Camera::init() noexcept {
	return impl_->init(cfg_.designator.c_str(), cfg_.connectionTimeout,
					   cfg_.rebootOnConnection);
}

bool Camera::init(const std::string& designator) noexcept {
	cfg_.designator = designator;
	return init();
}

bool Camera::init(const char* designator) noexcept {
	cfg_.designator = designator;
	return init();
}

bool Camera::trigger(TriggerType typ) noexcept { return impl_->trigger(typ); }

bool Camera::waitAndTrigger(TriggerType typ) noexcept {
	return impl_->waitAndTrigger(cfg_.triggerTimeout, typ);
}

bool Camera::acquire() noexcept {
	return impl_->acquire(cfg_.acquisitionTimeout);
}

bool Camera::startAcquisition(size_t nImages) noexcept {
	return impl_->startAcquisition(nImages);
}

void Camera::stopAcquisition() noexcept { impl_->stopAcquisition(); }

bool Camera::isAcquiring() const noexcept { return impl_->isAcquiring(); }

bool Camera::isInitialized() const noexcept { return impl_->isInitialized(); }

bool Camera::isAttached() const noexcept { return impl_->isAttached(); }

std::optional<Image> Camera::getImage() noexcept { return impl_->getImage(); }

bool Camera::cmdExecute(const std::string& cmd) noexcept {
	return impl_->cmdExecute(cmd.c_str());
}

bool Camera::cmdExecute(const char* cmd) noexcept {
	return impl_->cmdExecute(cmd);
}

bool Camera::cmdIsDone(const std::string& cmd) noexcept {
	return impl_->cmdIsDone(cmd.c_str());
}

bool Camera::cmdIsDone(const char* cmd) noexcept {
	return impl_->cmdIsDone(cmd);
}

}  // namespace camera
}  // namespace beholder
