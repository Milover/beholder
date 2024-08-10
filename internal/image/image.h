#ifndef _BEHOLDER_IMAGE_H
#define _BEHOLDER_IMAGE_H

#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
#include "libbeholder.h"
extern "C" {
#endif

#ifdef __cplusplus
typedef beholder::Processor* Proc;
typedef beholder::Tesseract* Tess;	// FIXME: remove this
#else
typedef void* Proc;
typedef void* Tess;	// FIXME: remove this
#endif

bool Proc_DecodeImage(Proc p, void* buf, int bufSize, int flags);
void Proc_Delete(Proc p);
bool Proc_Init(Proc p, void** post, size_t nPost, void** pre, size_t nPre);
Proc Proc_New();
bool Proc_Postprocess(Proc p, Tess t);	// FIXME: remove Tess
bool Proc_Preprocess(Proc p);
bool Proc_ReceiveAcquisitionResult(Proc p, const void* result);
bool Proc_ReadImage(Proc p, const char* filename, int flags);
void Proc_ShowImage(Proc p, const char* title);
bool Proc_WriteAcquisitionResult(Proc p, const void* result, const char* filename);
bool Proc_WriteImage(Proc p, const char* filename);

#ifdef __cplusplus
} // end extern "C"
#endif

#endif // _BEHOLDER_IMAGE_H
