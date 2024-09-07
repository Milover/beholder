#ifndef _BEHOLDER_IMGPROC_OPS_H
#define _BEHOLDER_IMGPROC_OPS_H

#include "stdbool.h"

#ifdef __cplusplus
#include "libbeholder.h"
extern "C" {
#endif

#ifdef __cplusplus
typedef beholder::AddPadding* AdPad;
typedef beholder::AdaptiveThreshold* AdThresh;
typedef beholder::AutoCrop* AuCrp;
typedef beholder::AutoOrient* AuOrien;
typedef beholder::CLAHE* CLH;
typedef beholder::Crop* Crp;
typedef beholder::Deblur* Dblr;
typedef beholder::DivGaussianBlur* DivGaussBlur;
typedef beholder::DrawBoundingBoxes* DrawBB;
typedef beholder::DrawLabels* DrawLbl;
typedef beholder::EqualizeHistogram* EqHist;
typedef beholder::FastNlMeansDenoise* FNlMDenoise;
typedef beholder::GaussianBlur* GaussBlur;
typedef beholder::Grayscale* Gray;
typedef beholder::Invert* Inv;
typedef beholder::Landscape* LScape;
typedef beholder::MedianBlur* MedBlur;
typedef beholder::Morphology* Morph;
typedef beholder::NormalizeBrightnessContrast* NormBC;
typedef beholder::Rescale* Rsl;
typedef beholder::Resize* Rsz;
typedef beholder::ResizeToHeight* RszToH;
typedef beholder::Rotate* Rot;
typedef beholder::Threshold* Thresh;
typedef beholder::UnsharpMask* UnshMsk;
#else
typedef void* AdPad;
typedef void* AdThresh;
typedef void* AuCrp;
typedef void* AuOrien;
typedef void* CLH;
typedef void* Crp;
typedef void* Dblr;
typedef void* DivGaussBlur;
typedef void* DrawBB;
typedef void* DrawLbl;
typedef void* EqHist;
typedef void* FNlMDenoise;
typedef void* GaussBlur;
typedef void* Gray;
typedef void* Inv;
typedef void* LScape;
typedef void* MedBlur;
typedef void* Morph;
typedef void* NormBC;
typedef void* Rot;
typedef void* Rsl;
typedef void* Rsz;
typedef void* RszToH;
typedef void* Thresh;
typedef void* UnshMsk;
#endif

AdPad AdPad_New(int pad, double padV);
AdThresh AdThresh_New(double max, int sz, double cnst, int typ);
AuCrp AuCrp_New(int kSize, float txtHeight, float txtWidth, float padding, double padV);
AuOrien AuOrien_New(int kSize, float txtHeight, float txtWidth, float padding, double padV);
CLH CLH_New(float lim, int tRows, int tCols);
Crp Crp_New(int left, int top, int width, int height);
Dblr Dblr_New(int radius, int snr);
DivGaussBlur DivGaussBlur_New(float scale, float sX, float sY, int kW, int kH);
DrawBB DrawBB_New(float* color, int thickness);
DrawLbl DrawLbl_New(float* color, double fontScale, int thickness);
EqHist EqHist_New();
FNlMDenoise FNlMDenoise_New(float weight);
GaussBlur GaussBlur_New(int kW, int kH, float sX, float sY);
Gray Gray_New();
Inv Inv_New();
LScape LScape_New();
MedBlur MedBlur_New(int ksize);
Morph Morph_New(int kTyp, int kW, int kH, int typ, int iter);
NormBC NormBC_New(float clipLowPct, float clipHighPct);
Rot Rot_New(float angle);
Rsl Rsl_New(double scale);
Rsz Rsz_New(int width, int height);
RszToH RszToH_New(int height);
Thresh Thresh_New(float val, float maxVal, int typ);
UnshMsk UnshMsk_New(double sig, double thresh, double amount);

#ifdef __cplusplus
} // end extern "C"
#endif

#endif // _BEHOLDER_IMGPROC_OPS_H
