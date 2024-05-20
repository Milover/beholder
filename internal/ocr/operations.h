#ifndef _PROCESSING_OPS_H
#define _PROCESSING_OPS_H

#ifdef __cplusplus
#include "libocr.h"
extern "C" {
#endif

#ifdef __cplusplus
typedef ocr::AddPadding* AdPad;
typedef ocr::AutoCrop* AuCrp;
typedef ocr::CLAHE* CLH;
typedef ocr::Crop* Crp;
typedef ocr::DivGaussianBlur* DivGaussBlur;
typedef ocr::DrawTextBoxes* DrawTB;
typedef ocr::EqualizeHistogram* EqHist;
typedef ocr::GaussianBlur* GaussBlur;
typedef ocr::Invert* Inv;
typedef ocr::MedianBlur* MedBlur;
typedef ocr::Morphology* Morph;
typedef ocr::NormalizeBrightnessContrast* NormBC;
typedef ocr::Resize* Rsz;
typedef ocr::Rotate* Rot;
typedef ocr::Threshold* Thresh;
#else
typedef void* AdPad;
typedef void* AuCrp;
typedef void* CLH;
typedef void* Crp;
typedef void* DivGaussBlur;
typedef void* DrawTB;
typedef void* EqHist;
typedef void* GaussBlur;
typedef void* Inv;
typedef void* MedBlur;
typedef void* Morph;
typedef void* NormBC;
typedef void* Rot;
typedef void* Rsz;
typedef void* Thresh;
#endif

AdPad AdPad_New(int pad);
AuCrp AuCrp_New(int kSize, float txtHeight, float txtWidth, float padding);
CLH CLH_New(float lim, int tRows, int tCols);
Crp Crp_New(int left, int top, int width, int height);
DivGaussBlur DivGaussBlur_New(float scale, float sX, float sY, int kW, int kH);
DrawTB DrawTB_New(float* color, int thickness);
EqHist EqHist_New();
GaussBlur GaussBlur_New(int kW, int kH, float sX, float sY);
Inv Inv_New();
MedBlur MedBlur_New(int ksize);
Morph Morph_New(int kTyp, int kW, int kH, int typ, int iter);
NormBC NormBC_New(float clipPct);
Rot Rot_New(float angle);
Rsz Rsz_New(int width, int height);
Thresh Thresh_New(float val, float maxVal, int typ);

#ifdef __cplusplus
} // end extern "C"
#endif

#endif // _OCR_H
