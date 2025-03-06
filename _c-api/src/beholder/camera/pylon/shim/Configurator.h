// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BEHOLDER_PYLON_SHIM_CONFIGURATOR_H
#define BEHOLDER_PYLON_SHIM_CONFIGURATOR_H

#include <GenApi/INodeMap.h>
#include <pylon/ConfigurationEventHandler.h>

namespace Pylon {
class CInstantCamera;
}  // namespace Pylon

namespace beholder {
namespace pylon {
namespace shim {

// Configurator is the default configuration used for all camera devices
// when they are initialized.
class Configurator : public Pylon::CConfigurationEventHandler {
protected:
	// Apply the configuration.
	virtual void applyConfiguration(GenApi::INodeMap& nodemap) const;

public:
	Configurator() = default;

	Configurator(const Configurator&) = default;
	Configurator(Configurator&&) = default;

	Configurator& operator=(const Configurator&) = default;
	Configurator& operator=(Configurator&&) = default;

	~Configurator() override = default;

	// Apply configuration right after the camera device is opened.
	void OnOpened(Pylon::CInstantCamera& cam) override;
};

}  // namespace shim
}  // namespace pylon
}  // namespace beholder

#endif	// BEHOLDER_PYLON_SHIM_CONFIGURATOR_H
