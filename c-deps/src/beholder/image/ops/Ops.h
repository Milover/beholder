// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

// Headers for image processing operations, collected for ease of use.

#ifndef BEHOLDER_IMAGE_OPS_OPS_H
#define BEHOLDER_IMAGE_OPS_OPS_H

#include "image/ops/AdaptiveThreshold.h"
#include "image/ops/AddPadding.h"
#include "image/ops/AutoCrop.h"
#include "image/ops/AutoOrient.h"
#include "image/ops/BGR.h"
#include "image/ops/CLAHE.h"
#include "image/ops/CorrectGamma.h"
#include "image/ops/Crop.h"
#include "image/ops/Deblur.h"
#include "image/ops/DivGaussianBlur.h"
#include "image/ops/DrawBoundingBoxes.h"
#include "image/ops/DrawLabels.h"
#include "image/ops/EqualizeHistogram.h"
#include "image/ops/FastNlMeansDenoise.h"
#include "image/ops/GaussianBlur.h"
#include "image/ops/Grayscale.h"
#include "image/ops/Invert.h"
#include "image/ops/Landscape.h"
#include "image/ops/MedianBlur.h"
#include "image/ops/Morphology.h"
#include "image/ops/NormalizeBrightnessContrast.h"
#include "image/ops/Rescale.h"
#include "image/ops/Resize.h"
#include "image/ops/ResizeToHeight.h"
#include "image/ops/Rotate.h"
#include "image/ops/Threshold.h"
#include "image/ops/UnsharpMask.h"

#endif	// BEHOLDER_IMAGE_OPS_OPS_H
