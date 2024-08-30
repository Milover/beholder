#include <chrono>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>
#include <string>

#include <pylon/Device.h>
#include <pylon/TypeMappings.h>

#include "camera.h"

bool Cam_Acquire(Cam c, size_t timeoutMs) {
	if (!c) {
		return false;
	}
	try {
		return c->acquire(std::chrono::milliseconds {timeoutMs});
	} catch(const Pylon::GenericException& e) {
		std::cerr << "could not acquire image: " << e.what() << std::endl;
	} catch(const beholder::Exception& e) {
		std::cerr << "could not acquire image: " << e.what() << std::endl;
	} catch(...) {
		std::cerr << "could not acquire image" << std::endl;
	}
	return false;
}

bool Cam_CmdExecute(Cam c, const char* cmd) {
	if (!c) {
		return false;
	}
	return c->cmdExecute(cmd);
}

bool Cam_CmdIsDone(Cam c, const char* cmd) {
	if (!c) {
		return false;
	}
	return c->cmdIsDone(cmd);
}

void Cam_Delete(Cam* c) {
	if (*c) {
		delete *c;
		*c = nullptr;
	}
}

Img Cam_GetRawImage(Cam c) {
	if (!c) {
		return Img {};
	}
	auto res {c->getRawImage()};
	if (!res) {
		return Img {};
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

bool Cam_Init(Cam c, const char* sn, Par* pars, size_t nPars, Trans t) {
	if (!c || !t) {
		return false;
	}
	// create device
	Pylon::IPylonDevice* d {
		t->createDevice(sn, beholder::DeviceDesignator::SN)
	};
	// initialize
	if (!d || !c->init(d)) {
		return false;
	}
	// set params
	beholder::ParamList list;	// OPTIMIZE: could avoid copying here
	list.reserve(nPars);
	for (auto i {0ul}; i < nPars; ++i) {
		list.emplace_back(pars[i].name, pars[i].value);
	}
	// NOTE: we don't technically have to fail
	return c->setParams(list);
}

Cam Cam_New() {
	return new beholder::Camera {};
}

bool Cam_SetParameters(Cam c, Par* pars, size_t nPars) {
	if (!c) {
		return false;
	}
	// set params
	beholder::ParamList list;	// OPTIMIZE: could avoid copying here
	list.reserve(nPars);
	for (auto i {0ul}; i < nPars; ++i) {
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

bool Cam_WaitAndTrigger(Cam c, size_t timeoutMs) {
	if (!c) {
		return false;
	}
	return c->waitAndTrigger(std::chrono::milliseconds {timeoutMs});
}

Pyl Pyl_New() {
	return new beholder::PylonAPI {};
}

void Pyl_Delete(Pyl* p) {
	if (*p) {
		delete *p;
		*p = nullptr;
	}
}

void Trans_Delete(Trans* t) {
	if (*t) {
		try {
			delete *t;
			*t = nullptr;
		} catch(...) {
			// XXX: ignore exceptions?
		}
	}
}

char* Trans_GetFirstSN(Trans t) {
	if (!t) {
		return nullptr;
	}
	std::string d {t->getFirstSN()};
	char* sn {new char[d.size() + 1]};
	std::strcpy(sn, d.c_str());
	return sn;
}

bool Trans_Init(Trans t, int dTyp) {
	if (!t) {
		return false;
	}
	return t->init(static_cast<beholder::DeviceClass>(dTyp));
}

Trans Trans_New() {
	return new beholder::TransportLayer {};
}
