#ifndef _OCR_H
#define _OCR_H

#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
#include "libocr.h"
extern "C" {
#endif

#ifdef __cplusplus
typedef ocr::ImageProcessor* Proc;
typedef ocr::Tesseract* Tess;
#else
typedef void* Proc;
typedef void* Tess;
#endif

typedef struct {
	char* key;
	char* value;
} KeyVal;

bool Proc_DecodeImage(Proc p, void* buf, int bufSize, int flags);
void Proc_Delete(Proc p);
bool Proc_Init(Proc p, void** post, size_t nPost, void** pre, size_t nPre);
Proc Proc_New();
bool Proc_Postprocess(Proc p, Tess t);
bool Proc_Preprocess(Proc p);
bool Proc_ReadImage(Proc p, const char* filename, int flags);
bool Proc_CopyBayerRGGB8(Proc p, int rows, int cols, void* buf, size_t step);
void Proc_ShowImage(Proc p, const char* title);
bool Proc_WriteImage(Proc p, const char* filename);

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
char* Tess_DetectAndRecognize(Tess t);
bool Tess_Init(Tess t, const TInit* in);
Tess Tess_New();
void Tess_SetImage(Tess t, Proc p, int bytesPerPixel);

#ifdef __cplusplus
} // end extern "C"
#endif

#endif // _OCR_H
