#ifndef _OCR_H
#define _OCR_H

#include <stdbool.h>

#ifdef __cplusplus
#include "libocr.h"
extern "C" {
#endif

#ifdef __cplusplus
typedef ocr::Config* Cfg;
typedef ocr::ImageProcessor* Proc;
typedef ocr::Tesseract* Tess;
#else
typedef void* Cfg;
typedef void* Proc;
typedef void* Tess;
#endif

Cfg Cfg_New(const char* json);

Proc Proc_New();
bool Proc_Preprocess(Proc p, Cfg cfg);
bool Proc_ReadImage(Proc p, const char* filename, int flags);
void Proc_ShowImage(Proc p, const char* title);

void Tess_Clear(Tess t);
bool Tess_DetectText(Tess t, Proc p, Cfg cfg);
Tess Tess_New(Cfg cfg);
char* Tess_RecognizeText(Tess t);
void Tess_SetImage(Tess t, Proc p, int bytesPerPixel);


#ifdef __cplusplus
} // end extern "C"
#endif

#endif // OCR_H
