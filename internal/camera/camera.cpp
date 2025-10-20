// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include "camera.h"

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

bool Cam_Acquire(Cam c) {
	if (!c) {
		return false;
	}
	return c->acquire();
}

bool Cam_CmdExecute(Cam c, const char *cmd) {
	if (!c) {
		return false;
	}
	return c->cmdExecute(cmd);
}

bool Cam_CmdIsDone(Cam c, const char *cmd) {
	if (!c) {
		return false;
	}
	return c->cmdIsDone(cmd);
}

void Cam_Delete(Cam *c) {
	if (*c) {
		delete *c;
		*c = nullptr;
	}
}

Img Cam_GetRawImage(Cam c) {
	if (!c) {
		return Img{};
	}
	auto res{c->getImage()};
	if (!res) {
		return Img{};
	}
	return std::move(res).value().moveToC();
}

bool Cam_IsAcquiring(Cam c) {
	if (c) {
		return c->isAcquiring();
	}
	return false;
}

bool Cam_IsAttached(Cam c) {
	if (c) {
		return c->isAttached();
	}
	return false;
}

bool Cam_IsInitialized(Cam c) {
	if (c) {
		return c->isInitialized();
	}
	return false;
}

bool Cam_Init(Cam c, const Cfg *cfg, Par *pars, size_t nPars) {
	if (!c || !cfg) {
		return false;
	}
	auto ms = [](int64_t ns) -> std::chrono::milliseconds {
		return std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::nanoseconds{ns});
	};
	// initialize
	if (!c->init(beholder::camera::Config{
			//.backend{},
			.deviceClass{
				beholder::enums::from<beholder::camera::DeviceClass>(cfg->dc)},
			.designator{cfg->sn},
			.acquisitionTimeout{ms(cfg->acqTimeout)},
			.triggerTimeout{ms(cfg->trgTimeout)},
			//.connectionTimeout{},
			.rebootOnConnection{cfg->reboot},
		})) {
		return false;
	}
	return Cam_SetParameters(c, pars, nPars);
}

Cam Cam_New() { return new beholder::camera::Camera{}; }

bool Cam_SetParameters(Cam c, Par *pars, size_t nPars) {
	if (!c) {
		return false;
	}
	// set params
	beholder::camera::ParamVector list;	 // OPTIMIZE: could avoid copying here
	list.reserve(nPars);
	for (auto i{0ul}; i < nPars; ++i) {
		list.emplace_back(pars[i].name, pars[i].value);
	}
	// NOTE: we don't technically have to fail
	return c->setParams(list);
}

bool Cam_StartAcquisition(Cam c) {
	if (c) {
		return c->startAcquisition();
	}
	return false;
}

void Cam_StopAcquisition(Cam c) {
	if (c) {
		c->stopAcquisition();
	}
}

bool Cam_Trigger(Cam c) {
	if (!c) {
		return false;
	}
	return c->trigger();
}

bool Cam_WaitAndTrigger(Cam c) {
	if (!c) {
		return false;
	}
	return c->waitAndTrigger();
}
