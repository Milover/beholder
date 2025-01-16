// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BEHOLDER_CAMERA_INTERNAL_DEFAULT_CONFIGURATOR_H
#define BEHOLDER_CAMERA_INTERNAL_DEFAULT_CONFIGURATOR_H

#include <GenApi/INodeMap.h>
#include <pylon/ConfigurationEventHandler.h>
#include <pylon/InstantCamera.h>

namespace beholder {
namespace internal {

// DefaultConfigurator is the default configuration used for all camera devices
// when they are initialized.
class DefaultConfigurator : public Pylon::CConfigurationEventHandler {
protected:
	// Apply the configuration.
	virtual void applyConfiguration(GenApi::INodeMap& nodemap) const;

public:
	DefaultConfigurator() = default;

	DefaultConfigurator(const DefaultConfigurator&) = default;
	DefaultConfigurator(DefaultConfigurator&&) = default;

	~DefaultConfigurator() override = default;

	DefaultConfigurator& operator=(const DefaultConfigurator&) = default;
	DefaultConfigurator& operator=(DefaultConfigurator&&) = default;

	// Apply configuration right after the camera device is opened.
	void OnOpened(Pylon::CInstantCamera& cam) override;
};

}  // namespace internal
}  // namespace beholder

#endif	// BEHOLDER_CAMERA_INTERNAL_DEFAULT_CONFIGURATOR_H
