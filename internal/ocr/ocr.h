#ifndef _BEHOLDER_OCR_H
#define _BEHOLDER_OCR_H

#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
#include "libbeholder.h"
extern "C" {
#endif

#ifdef __cplusplus
typedef beholder::Processor* Proc;
typedef beholder::Tesseract* Tess;
#else
typedef void* Proc;
typedef void* Tess;
#endif

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
void Tess_SetImage(Tess t, Proc p, int bytesPerPixel);

#ifdef __cplusplus
} // end extern "C"
#endif

#endif // _BEHOLDER_OCR_H
