#ifndef _BEHOLDER_IMGPROC_H
#define _BEHOLDER_IMGPROC_H

#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
#include "libbeholder.h"
extern "C" {
#else
#include "RawImage.h"
#include "Result.h"
#endif

#ifdef __cplusplus
typedef beholder::Processor* Proc;
typedef beholder::capi::RawImage Img;
typedef beholder::capi::Result Res;
#else
typedef void* Proc;
typedef RawImage Img;
typedef Result Res;
#endif

bool Proc_DecodeImage(Proc p, void* buf, int bufSize, int flags);
void Proc_Delete(Proc p);
Img Proc_GetRawImage(Proc p);
bool Proc_Init(Proc p, void** post, size_t nPost, void** pre, size_t nPre);
Proc Proc_New();
bool Proc_Postprocess(Proc p, Res* res, size_t nRes);
bool Proc_Preprocess(Proc p);
bool Proc_ReceiveRawImage(Proc p, const Img* img);
bool Proc_ReadImage(Proc p, const char* filename, int flags);
void Proc_ShowImage(Proc p, const char* title);
bool Proc_WriteImage(Proc p, const char* filename);

#ifdef __cplusplus
} // end extern "C"
#endif

#endif // _BEHOLDER_IMGPROC_H
