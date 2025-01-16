#ifndef _BEHOLDER_NEURAL_SHIM_H
#define _BEHOLDER_NEURAL_SHIM_H

#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
#include <beholder/Beholder.h>
extern "C" {
#else
#include <beholder/capi/Image.h>
#include <beholder/capi/Result.h>
#endif

#ifdef __cplusplus
typedef beholder::ObjDetector* Det;
typedef beholder::Tesseract* Tess;
typedef beholder::capi::Image Img;
typedef beholder::capi::Result Res;
#else
typedef void* Det;
typedef void* Tess;
typedef Image Img;
typedef Result Res;
#endif

typedef struct {
	Res* array;
	size_t count;
} ResArr;  // deallocation helper

void ResArr_Delete(void* r);

typedef struct {
	const char* modelPath;
	const char* model;
	int backend;
	int target;
	int size[2];
	double scale[3];
	float conf;
	float nms;
	double mean[3];
	bool swapRB;
	double pad[3];
} DetInit;

// do stuff with a detector
void Det_Clear(Det d);
void Det_Delete(Det d);
ResArr* Det_Detect(Det d, const Img* img);
bool Det_Init(Det d, const DetInit* in);
// allocate new detectors
Det Det_NewCRAFT();
Det Det_NewEAST();
Det Det_NewPARSeq();
Det Det_NewYOLOv8();
// configure a specific detector
bool Det_ConfigureCRAFT(Det d, float txtThresh, float lnThresh, float lowTxt);
bool Det_ConfigurePARSeq(Det d, const char* charset);
bool Det_ConfigureYOLOv8(Det d, const char** classes, size_t nClasses);

typedef struct {
	char* key;
	char* value;
} KeyVal;

typedef struct {
	char** cfgs;
	size_t nCfgs;
	const char* modelPath;
	const char* model;
	int psMode;
	KeyVal* vars;  // runtime settable Tesseract variables
	size_t nVars;
} TInit;

void Tess_Clear(Tess t);
void Tess_Delete(Tess t);
ResArr* Tess_Recognize(Tess t);
bool Tess_Init(Tess t, const TInit* in);
Tess Tess_New();
bool Tess_SetImage(Tess t, const Img* img);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif	// _BEHOLDER_NEURAL_SHIM_H
