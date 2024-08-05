#ifndef _BEHOLDER_CAMERA_H
#define _BEHOLDER_CAMERA_H

#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
#include <pylon/GrabResultPtr.h>

#include "libbeholder.h"
extern "C" {
#endif

#ifdef __cplusplus
typedef beholder::Camera* Cam;
typedef beholder::PylonAPI* Pyl;
typedef beholder::TransportLayer* Trans;
typedef const Pylon::CGrabResultPtr* Res;
#else
typedef void* Cam;
typedef void* Pyl;
typedef void* Trans;
typedef const void* Res;
#endif

typedef struct {
	char* name;
	char* value;
} Par;

typedef struct {
	Res ptr;	// NOTE: weak pointer; no need to call free
	size_t id;
} Result;

bool Cam_Acquire(Cam c, size_t timeoutMs);
void Cam_Delete(Cam* c);
Result Cam_GetResult(Cam c);
bool Cam_IsAcquiring(Cam c);
bool Cam_IsAttached(Cam c);
bool Cam_IsInitialized(Cam c);
bool Cam_Init(Cam c, const char* sn, Par* pars, size_t nPars, Trans t);
Cam Cam_New();
// TODO: bool Cam_SetParameters(Params p)
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
