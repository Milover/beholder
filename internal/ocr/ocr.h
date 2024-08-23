#ifndef _BEHOLDER_OCR_H
#define _BEHOLDER_OCR_H

#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
#include "libbeholder.h"
extern "C" {
#endif

#ifdef __cplusplus
typedef beholder::DBDetector* DB;
typedef beholder::EASTDetector* EAST;
typedef beholder::Detector* Det;
typedef beholder::Tesseract* Tess;
#else
typedef void* DB;
typedef void* EAST;
typedef void* Det;
typedef void* Tess;
#endif

// FIXME: we're copying this struct in like 7 places
typedef struct {
	size_t id;
	int rows;
	int cols;
	int64_t pxTyp;
	void* buf;	// NOTE: weak pointer; no need to call free
	size_t step;
} RawImage;

typedef struct {
	int left, top, right, bottom;
} Rect;

typedef struct {
	char* text;
	double conf;
	Rect box;
} Res;		// per-line result

typedef struct {
	Res* array;
	size_t count;
} ResArr;	// deallocation helper

void ResArr_Delete(void* r);

typedef struct {
	const char* modelPath;
	const char* model;
	float binary;
	float polygon;
	int maxCand;
	double unclip;
	bool useHCMean;
} DBInit;

bool DB_Init(DB d, const DBInit* in);
DB DB_New();

void Det_Clear(Det d);
void Det_Delete(Det d);
ResArr* Det_Detect(Det d, const RawImage* img);

typedef struct {
	const char* modelPath;
	const char* model;
	float conf;
	float nms;
	bool useHCMean;
} EASTInit;

bool EAST_Init(EAST e, const EASTInit* in);
EAST EAST_New();

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
	KeyVal* vars;	// runtime settable Tesseract variables
	size_t nVars;
} TInit;

void Tess_Clear(Tess t);
void Tess_Delete(Tess t);
ResArr* Tess_Recognize(Tess t);
bool Tess_Init(Tess t, const TInit* in);
Tess Tess_New();
bool Tess_SetImage(Tess t, const RawImage* img, int bytesPerPixel);

#ifdef __cplusplus
} // end extern "C"
#endif

#endif // _BEHOLDER_OCR_H
