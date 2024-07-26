#ifndef _CAMERA_H
#define _CAMERA_H

#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
#include "libcamera.h"
extern "C" {
#endif

#ifdef __cplusplus
typedef camera::Camera* Cam;
typedef camera::Image* Img;
typedef camera::PylonAPI* Pyl;
typedef camera::TransportLayer* Trans;
#else
typedef void* Cam;
typedef void* Img;
typedef void* Pyl;
typedef void* Trans;
#endif

typedef struct {
	char* name;
	char* value;
} Par;

bool Cam_Acquire(Cam c, Img* i, size_t timeoutMs);
void Cam_Delete(Cam* c);
bool Cam_IsAcquiring(Cam c);
bool Cam_IsAttached(Cam c);
bool Cam_IsInitialized(Cam c);
bool Cam_Init(Cam c, const char* macAddr, Par* pars, size_t nPars, Trans t);
Cam Cam_New();
// TODO: bool Cam_SetParameters(Params p)
bool Cam_StartAcquisition(Cam c);
void Cam_StopAcquisition(Cam c);
bool Cam_Trigger(Cam c);
bool Cam_WaitAndTrigger(Cam c, size_t timeoutMs);

typedef struct {
	size_t id;
	size_t cols;
	size_t rows;
	size_t step;
	bool mono;
} ImgInfo;

unsigned char* Img_Buffer(Img i);
void Img_Delete(Img i);
bool Img_Write(Img i, const char* filename);
ImgInfo* Img_Info(Img i);

Pyl Pyl_New();
void Pyl_Delete(Pyl* p);

void Trans_Delete(Trans* t);
bool Trans_Init(Trans t);
Trans Trans_New();

#ifdef __cplusplus
} // end extern "C"
#endif

#endif // _CAMERA_H
