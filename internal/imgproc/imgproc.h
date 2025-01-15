#ifndef _BEHOLDER_IMAGE_SHIM_H
#define _BEHOLDER_IMAGE_SHIM_H

#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
#include <beholder/Beholder.h>
extern "C" {
#else
#include <beholder/capi/Image.h>
#include <beholder/capi/Rectangle.h>
#include <beholder/capi/Result.h>
#endif

#ifdef __cplusplus
typedef beholder::Processor* Proc;
typedef beholder::capi::Image Img;
typedef beholder::capi::Rectangle Rect;
typedef beholder::capi::Result Res;
#else
typedef void* Proc;
typedef Image Img;
typedef Rectangle Rect;
typedef Result Res;
#endif

bool Proc_DecodeImage(Proc p, void* buf, int bufSize, int flags);
void Proc_Delete(Proc p);
const unsigned char* Proc_EncodeImage(Proc p, const char* ext, int* encSize);
Img Proc_GetRawImage(Proc p);
bool Proc_Init(Proc p, void** post, size_t nPost, void** pre, size_t nPre);
Proc Proc_New();
bool Proc_Postprocess(Proc p, Res* res, size_t nRes);
bool Proc_Preprocess(Proc p);
bool Proc_ReceiveRawImage(Proc p, const Img* img);
bool Proc_ReadImage(Proc p, const char* filename, int flags);
void Proc_ResetROI(Proc p);
void Proc_SetROI(Proc p, const Rect* roi);
void Proc_SetRotatedROI(Proc p, const Rect* roi, double ang);
void Proc_ShowImage(Proc p, const char* title);
void Proc_ToColor(Proc p);
void Proc_ToGrayscale(Proc p);
bool Proc_WriteImage(Proc p, const char* filename);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif	// _BEHOLDER_IMAGE_SHIM_H
