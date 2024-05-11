#include <array>

#include "processing_ops.h"

DrawTB DrawTB_New(float color[4], int thickness) {
	std::array<float, 4> c {color[0], color[1], color[2], color[3]};
	return new ocr::DrawTextBoxes {c, thickness};
}

GaussBlur GaussBlur_New(int kW, int kH, float sX, float sY) {
	return new ocr::GaussianBlur {kW, kH, sX, sY};
}

Inv Inv_New() {
	return new ocr::Invert {};
}

MedBlur MedBlur_New(int ksize) {
	return new ocr::MedianBlur {ksize};
}

Morph Morph_New(int kTyp, int kW, int kH, int typ, int iter) {
	return new ocr::Morphology {kTyp, kW, kH, typ, iter};
}

NormBC NormBC_New(float clipPct) {
	return new ocr::NormalizeBrightnessContrast {clipPct};
}

Rsz Rsz_New(int width, int height) {
	return new ocr::Resize {width, height};
}

Thresh Thresh_New(float val, float maxVal, int typ) {
	return new ocr::Threshold {val, maxVal, typ};
}
