#include <chrono>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <vector>
#include <string>

#include <pylon/TypeMappings.h>
#include <pylon/Device.h>

#include "camera.h"

bool Cam_Acquire(Cam c, Img* i, size_t timeoutMs) {
	if (!c) {
		return false;
	}
	if (*i) {
		delete (*i);
		*i = nullptr;
	}
	try {
		std::unique_ptr<camera::Image> img {
			c->acquire(std::chrono::milliseconds {timeoutMs})
		};
		if (img) {
			*i = img.release();
		}
		return true;
	} catch(const Pylon::GenericException& e) {
		std::cerr << "could not acquire image: " << e.what() << std::endl;
	} catch(const camera::Exception& e) {
		std::cerr << "could not acquire image: " << e.what() << std::endl;
	} catch(...) {
		std::cerr << "could not acquire image" << std::endl;
	}
	return false;
}

void Cam_Delete(Cam* c) {
	if (*c) {
		delete *c;
		*c = nullptr;
	}
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

bool Cam_Init(Cam c, const char* macAddr, Par* pars, size_t nPars, Trans t) {
	if (!c || !t) {
		return false;
	}
	// create device
	Pylon::IPylonDevice* d {
		t->createDevice(macAddr, camera::DeviceDesignator::MAC)
	};
	if (!d) {
		return false;
	}
	// initialize
	if (!c->init(d)) {
		return false;
	}
	// set params
	camera::ParamList list;
	list.reserve(nPars);
	for (auto i {0ul}; i < nPars; ++i) {
		list.emplace_back(pars[i].name, pars[i].value);
	}
	// NOTE: we don't technically have to fail
	return c->setParams(list);
}

Cam Cam_New() {
	return new camera::Camera {};
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

// TODO: bool Cam_SetParameters(Params p)

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

unsigned char* Img_Buffer(Img i) {
	if (!i) {
		return nullptr;
	}
	return i->getBuffer();
}

ImgInfo* Img_Info(Img i) {
	ImgInfo* info;
	if (i) {
		info = new ImgInfo {i->id, i->cols, i->rows, i->getStep(), i->isMonochrome()};
	}
	return info;
}

void Img_Delete(Img i) {
	if (i) {
		delete i;
		i = nullptr;
	}
}

bool Img_Write(Img i, const char* filename) {
	if (!i) {
		return false;
	}
	std::string s {filename};
	return i->write(s);
}

Pyl Pyl_New() {
	return new camera::PylonAPI {};
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

bool Trans_Init(Trans t) {
	if (!t) {
		return false;
	}
	return t->init();
}

Trans Trans_New() {
	return new camera::TransportLayer {};
}
