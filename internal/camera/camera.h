// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#ifndef _BEHOLDER_CAMERA_SHIM_H
#define _BEHOLDER_CAMERA_SHIM_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
#include <beholder/camera/BeholderCamera.h>
extern "C" {
#else
#include <beholder/capi/Image.h>
#endif

#ifdef __cplusplus
typedef beholder::camera::Camera* Cam;
typedef beholder::capi::Image Img;
#else
typedef void* Cam;
typedef Image Img;
#endif

typedef struct {
	char* name;
	char* value;
} Par;

typedef struct {
	int32_t bnd;		 // backend
	int32_t dc;			 // device class
	const char* sn;		 // serial number
	int64_t acqTimeout;	 // acquisition timeout in ns
	int64_t trgTimeout;	 // trigger timeout in ns
	int64_t conTimeout;	 // connection timeout in ns
	bool reboot;
} Cfg;

bool Cam_Acquire(Cam c);
bool Cam_CmdExecute(Cam c, const char* cmd);
bool Cam_CmdIsDone(Cam c, const char* cmd);
void Cam_Delete(Cam* c);
Img Cam_GetRawImage(Cam c);
bool Cam_IsAcquiring(Cam c);
bool Cam_IsAttached(Cam c);
bool Cam_IsInitialized(Cam c);
bool Cam_Init(Cam c, const Cfg* cfg, Par* pars, size_t nPars);
Cam Cam_New();
bool Cam_SetParameters(Cam c, Par* pars, size_t nPars);
bool Cam_StartAcquisition(Cam c);
void Cam_StopAcquisition(Cam c);
bool Cam_Trigger(Cam c);
bool Cam_WaitAndTrigger(Cam c);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif	// _BEHOLDER_CAMERA_SHIM_H
