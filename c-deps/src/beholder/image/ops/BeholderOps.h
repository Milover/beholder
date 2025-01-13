// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

// Headers for image processing operations, collected for ease of use.

#ifndef BEHOLDER_IMAGE_OPS_H
#define BEHOLDER_IMAGE_OPS_H

#include "beholder/image/ops/AdaptiveThreshold.h"
#include "beholder/image/ops/AddPadding.h"
#include "beholder/image/ops/AutoCrop.h"
#include "beholder/image/ops/AutoOrient.h"
#include "beholder/image/ops/BGR.h"
#include "beholder/image/ops/CLAHE.h"
#include "beholder/image/ops/CorrectGamma.h"
#include "beholder/image/ops/Crop.h"
#include "beholder/image/ops/Deblur.h"
#include "beholder/image/ops/DivGaussianBlur.h"
#include "beholder/image/ops/DrawBoundingBoxes.h"
#include "beholder/image/ops/DrawLabels.h"
#include "beholder/image/ops/EqualizeHistogram.h"
#include "beholder/image/ops/FastNlMeansDenoise.h"
#include "beholder/image/ops/GaussianBlur.h"
#include "beholder/image/ops/Grayscale.h"
#include "beholder/image/ops/Invert.h"
#include "beholder/image/ops/Landscape.h"
#include "beholder/image/ops/MedianBlur.h"
#include "beholder/image/ops/Morphology.h"
#include "beholder/image/ops/NormalizeBrightnessContrast.h"
#include "beholder/image/ops/Rescale.h"
#include "beholder/image/ops/Resize.h"
#include "beholder/image/ops/ResizeToHeight.h"
#include "beholder/image/ops/Rotate.h"
#include "beholder/image/ops/Threshold.h"
#include "beholder/image/ops/UnsharpMask.h"

#endif	// BEHOLDER_IMAGE_OPS_H
