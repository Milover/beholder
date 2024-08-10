#ifndef _BEHOLDER_IMAGE_H
#define _BEHOLDER_IMAGE_H

#ifdef __cplusplus
#include "libbeholder.h"
extern "C" {
#endif

#ifdef __cplusplus
typedef beholder::AddPadding* AdPad;
typedef beholder::AutoCrop* AuCrp;
typedef beholder::CLAHE* CLH;
typedef beholder::Crop* Crp;
typedef beholder::DivGaussianBlur* DivGaussBlur;
typedef beholder::DrawBoundingBoxes* DrawBB;
typedef beholder::EqualizeHistogram* EqHist;
typedef beholder::GaussianBlur* GaussBlur;
typedef beholder::Invert* Inv;
typedef beholder::MedianBlur* MedBlur;
typedef beholder::Morphology* Morph;
typedef beholder::NormalizeBrightnessContrast* NormBC;
typedef beholder::Resize* Rsz;
typedef beholder::Rotate* Rot;
typedef beholder::Threshold* Thresh;
#else
typedef void* AdPad;
typedef void* AuCrp;
typedef void* CLH;
typedef void* Crp;
typedef void* DivGaussBlur;
typedef void* DrawBB;
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
DrawBB DrawBB_New(float* color, int thickness);
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

#endif // _BEHOLDER_IMAGE_H
