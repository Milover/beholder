#ifndef _BEHOLDER_CAMERA_H
#define _BEHOLDER_CAMERA_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
#include "libbeholder_camera.h"
extern "C" {
#endif

#ifdef __cplusplus
typedef beholder::Camera* Cam;
typedef beholder::PylonAPI* Pyl;
typedef beholder::TransportLayer* Trans;
#else
typedef void* Cam;
typedef void* Pyl;
typedef void* Trans;
#endif

typedef struct {
	char* name;
	char* value;
} Par;

// FIXME: we're copying this struct in like 7 places
typedef struct {
	size_t id;
	int rows;
	int cols;
	int64_t pxTyp;
	void* buf;	// NOTE: weak pointer; no need to call free
	size_t step;
} RawImage;

bool Cam_Acquire(Cam c, size_t timeoutMs);
bool Cam_CmdExecute(Cam c, const char* cmd);
bool Cam_CmdIsDone(Cam c, const char* cmd);
void Cam_Delete(Cam* c);
RawImage Cam_GetRawImage(Cam c);
bool Cam_IsAcquiring(Cam c);
bool Cam_IsAttached(Cam c);
bool Cam_IsInitialized(Cam c);
bool Cam_Init(Cam c, const char* sn, Par* pars, size_t nPars, Trans t);
Cam Cam_New();
bool Cam_SetParameters(Cam c, Par* pars, size_t nPars);
bool Cam_StartAcquisition(Cam c);
void Cam_StopAcquisition(Cam c);
bool Cam_Trigger(Cam c);
bool Cam_WaitAndTrigger(Cam c, size_t timeoutMs);

Pyl Pyl_New();
void Pyl_Delete(Pyl* p);

void Trans_Delete(Trans* t);
char* Trans_GetFirstSN(Trans t);
bool Trans_Init(Trans t, int dTyp);
Trans Trans_New();

#ifdef __cplusplus
} // end extern "C"
#endif

#endif // _CAMERA_H
