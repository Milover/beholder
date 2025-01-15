#include "ops.h"

#include <array>

AdPad AdPad_New(int pad, double padV) {
	return new beholder::AddPadding{pad, padV};
}

AdThresh AdThresh_New(double max, int sz, double cnst, int typ) {
	namespace be = beholder::enums;
	using Type = beholder::AdaptiveThreshold::Type;
	return new beholder::AdaptiveThreshold{max, sz, cnst, be::from<Type>(typ)};
}

AuCrp AuCrp_New(int kSize, float txtHeight, float txtWidth, float padding,
				double padV) {
	return new beholder::AutoCrop{kSize, txtHeight, txtWidth, padding, padV};
}

AuOrien AuOrien_New(int kSize, float txtHeight, float txtWidth, float padding,
					double padV) {
	return new beholder::AutoOrient{kSize, txtHeight, txtWidth, padding, padV};
}

BGR BGR_New() { return new beholder::BGR{}; }

CLH CLH_New(float lim, int tRows, int tCols) {
	return new beholder::CLAHE{lim, tRows, tCols};
}

CorrGamma CorrGamma_New(double gamma) {
	return new beholder::CorrectGamma{gamma};
}

Crp Crp_New(int left, int top, int width, int height) {
	return new beholder::Crop{left, top, width, height};
}

Dblr Dblr_New(int radius, int snr) { return new beholder::Deblur{radius, snr}; }

DivGaussBlur DivGaussBlur_New(float scale, float sX, float sY, int kW, int kH) {
	return new beholder::DivGaussianBlur{scale, sX, sY, kW, kH};
}

DrawBB DrawBB_New(float color[4], int thickness) {
	std::array<float, 4> c{color[0], color[1], color[2], color[3]};
	return new beholder::DrawBoundingBoxes{c, thickness};
}

DrawLbl DrawLbl_New(float* color, double fontScale, int thickness) {
	std::array<float, 4> c{color[0], color[1], color[2], color[3]};
	return new beholder::DrawLabels{c, fontScale, thickness};
}

EqHist EqHist_New() { return new beholder::EqualizeHistogram{}; }

FNlMDenoise FNlMDenoise_New(float weight) {
	return new beholder::FastNlMeansDenoise{weight};
}

GaussBlur GaussBlur_New(int kW, int kH, float sX, float sY) {
	return new beholder::GaussianBlur{kW, kH, sX, sY};
}

Gray Gray_New() { return new beholder::Grayscale{}; }

Inv Inv_New() { return new beholder::Invert{}; }

LScape LScape_New() { return new beholder::Landscape{}; }

MedBlur MedBlur_New(int ksize) { return new beholder::MedianBlur{ksize}; }

Morph Morph_New(int kTyp, int kW, int kH, int typ, int iter) {
	namespace be = beholder::enums;
	using Shape = beholder::Morphology::Shape;
	using Type = beholder::Morphology::Type;
	return new beholder::Morphology{be::from<Shape>(kTyp), kW, kH,
									be::from<Type>(typ), iter};
}

NormBC NormBC_New(float clipLowPct, float clipHighPct) {
	return new beholder::NormalizeBrightnessContrast{clipLowPct, clipHighPct};
}

Rot Rot_New(float angle) { return new beholder::Rotate{angle}; }

Rsl Rsl_New(double scale) { return new beholder::Rescale{scale}; }

Rsz Rsz_New(int width, int height) {
	return new beholder::Resize{width, height};
}

RszToH RszToH_New(int height) { return new beholder::ResizeToHeight{height}; }

Thresh Thresh_New(float val, float maxVal, int typ) {
	namespace be = beholder::enums;
	using Type = beholder::Threshold::Type;
	return new beholder::Threshold{val, maxVal, be::from<Type>(typ)};
}

UnshMsk UnshMsk_New(double sig, double thresh, double amount) {
	return new beholder::UnsharpMask{sig, thresh, amount};
}
