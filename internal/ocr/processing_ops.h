#ifndef _PROCESSING_OPS_H
#define _PROCESSING_OPS_H

#ifdef __cplusplus
#include "libocr.h"
extern "C" {
#endif

#ifdef __cplusplus
typedef ocr::DrawTextBoxes* DrawTB;
typedef ocr::GaussianBlur* GaussBlur;
typedef ocr::Invert* Inv;
typedef ocr::MedianBlur* MedBlur;
typedef ocr::Morphology* Morph;
typedef ocr::NormalizeBrightnessContrast* NormBC;
typedef ocr::Resize* Rsz;
typedef ocr::Threshold* Thresh;
#else
typedef void* DrawTB;
typedef void* GaussBlur;
typedef void* Inv;
typedef void* MedBlur;
typedef void* Morph;
typedef void* NormBC;
typedef void* Rsz;
typedef void* Thresh;
#endif

DrawTB DrawTB_New(float* color, int thickness);
GaussBlur GaussBlur_New(int kW, int kH, float sX, float sY);
Inv Inv_New();
MedBlur MedBlur_New(int ksize);
Morph Morph_New(int kTyp, int kW, int kH, int typ, int iter);
NormBC NormBC_New(float clipPct);
Rsz Rsz_New(int width, int height);
Thresh Thresh_New(float val, float maxVal, int typ);

#ifdef __cplusplus
} // end extern "C"
#endif

#endif // _OCR_H
