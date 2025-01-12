// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: MIT

// A helper class for holding information needed for receiving/converting
// camera acquisition results.

#ifndef BEHOLDER_IMAGE_CONVERSION_INFO_H
#define BEHOLDER_IMAGE_CONVERSION_INFO_H

#include <array>
#include <cstdint>
#include <optional>
#include <utility>

// Pylon macros for computing (part of) the pixel type ID.
#define PX_MONO (0x01000000)
#define PX_COLOR (0x02000000)
#define PX_CUSTOM (0x80000000)
#define PX_BIT_CNT(n) ((n) << 16)

namespace beholder {

class ConversionInfo {
public:
	int inputType;		// CvMat type of the (foreign) image buffer
	int outChannels;	// number of channels of the output image
	int colorConvCode;	// CvColor conversion code.
};

enum class PxType : int64_t {
	// Undefined = -1,
	Mono1packed = PX_CUSTOM | PX_MONO | PX_BIT_CNT(1) | 0x000c,
	Mono2packed = PX_CUSTOM | PX_MONO | PX_BIT_CNT(2) | 0x000d,
	Mono4packed = PX_CUSTOM | PX_MONO | PX_BIT_CNT(4) | 0x000e,
	Mono8 = PX_MONO | PX_BIT_CNT(8) | 0x0001,
	Mono8signed = PX_MONO | PX_BIT_CNT(8) | 0x0002,
	Mono10 = PX_MONO | PX_BIT_CNT(16) | 0x0003,
	Mono10packed = PX_MONO | PX_BIT_CNT(12) | 0x0004,
	Mono10p = PX_MONO | PX_BIT_CNT(10) | 0x0046,
	Mono12 = PX_MONO | PX_BIT_CNT(16) | 0x0005,
	Mono12packed = PX_MONO | PX_BIT_CNT(12) | 0x0006,
	Mono12p = PX_MONO | PX_BIT_CNT(12) | 0x0047,
	Mono16 = PX_MONO | PX_BIT_CNT(16) | 0x0007,
	BayerGR8 = PX_MONO | PX_BIT_CNT(8) | 0x0008,
	BayerRG8 = PX_MONO | PX_BIT_CNT(8) | 0x0009,
	BayerGB8 = PX_MONO | PX_BIT_CNT(8) | 0x000a,
	BayerBG8 = PX_MONO | PX_BIT_CNT(8) | 0x000b,
	BayerGR10 = PX_MONO | PX_BIT_CNT(16) | 0x000c,
	BayerRG10 = PX_MONO | PX_BIT_CNT(16) | 0x000d,
	BayerGB10 = PX_MONO | PX_BIT_CNT(16) | 0x000e,
	BayerBG10 = PX_MONO | PX_BIT_CNT(16) | 0x000f,
	BayerGR12 = PX_MONO | PX_BIT_CNT(16) | 0x0010,
	BayerRG12 = PX_MONO | PX_BIT_CNT(16) | 0x0011,
	BayerGB12 = PX_MONO | PX_BIT_CNT(16) | 0x0012,
	BayerBG12 = PX_MONO | PX_BIT_CNT(16) | 0x0013,
	RGB8packed = PX_COLOR | PX_BIT_CNT(24) | 0x0014,
	BGR8packed = PX_COLOR | PX_BIT_CNT(24) | 0x0015,
	RGBA8packed = PX_COLOR | PX_BIT_CNT(32) | 0x0016,
	BGRA8packed = PX_COLOR | PX_BIT_CNT(32) | 0x0017,
	RGB10packed = PX_COLOR | PX_BIT_CNT(48) | 0x0018,
	BGR10packed = PX_COLOR | PX_BIT_CNT(48) | 0x0019,
	RGB12packed = PX_COLOR | PX_BIT_CNT(48) | 0x001a,
	BGR12packed = PX_COLOR | PX_BIT_CNT(48) | 0x001b,
	RGB16packed = PX_COLOR | PX_BIT_CNT(48) | 0x0033,
	// BGR10V1packed = PX_COLOR | PX_BIT_CNT(32) | 0x001c,
	// BGR10V2packed = PX_COLOR | PX_BIT_CNT(32) | 0x001d,
	// YUV411packed = PX_COLOR | PX_BIT_CNT(12) | 0x001e,
	// YUV422packed = PX_COLOR | PX_BIT_CNT(16) | 0x001f,
	// YUV444packed = PX_COLOR | PX_BIT_CNT(24) | 0x0020,
	// RGB8planar = PX_COLOR | PX_BIT_CNT(24) | 0x0021,
	// RGB10planar = PX_COLOR | PX_BIT_CNT(48) | 0x0022,
	// RGB12planar = PX_COLOR | PX_BIT_CNT(48) | 0x0023,
	// RGB16planar = PX_COLOR | PX_BIT_CNT(48) | 0x0024,
	// YUV422_YUYV_Packed = PX_COLOR | PX_BIT_CNT(16) | 0x0032,
	// YUV444planar = PX_CUSTOM | PX_COLOR | PX_BIT_CNT(24) | 0x0044,
	// YUV422planar = PX_CUSTOM | PX_COLOR | PX_BIT_CNT(16) | 0x0042,
	// YUV420planar = PX_CUSTOM | PX_COLOR | PX_BIT_CNT(12) | 0x0040,
	// YCbCr420_8_YY_CbCr_Semiplanar = PX_COLOR | PX_BIT_CNT(12) | 0x0112,
	// YCbCr422_8_YY_CbCr_Semiplanar = PX_COLOR | PX_BIT_CNT(16) | 0x0113,
	BayerGR12Packed = PX_MONO | PX_BIT_CNT(12) | 0x002A,
	BayerRG12Packed = PX_MONO | PX_BIT_CNT(12) | 0x002B,
	BayerGB12Packed = PX_MONO | PX_BIT_CNT(12) | 0x002C,
	BayerBG12Packed = PX_MONO | PX_BIT_CNT(12) | 0x002D,
	BayerGR10p = PX_MONO | PX_BIT_CNT(10) | 0x0056,
	BayerRG10p = PX_MONO | PX_BIT_CNT(10) | 0x0058,
	BayerGB10p = PX_MONO | PX_BIT_CNT(10) | 0x0054,
	BayerBG10p = PX_MONO | PX_BIT_CNT(10) | 0x0052,
	BayerGR12p = PX_MONO | PX_BIT_CNT(12) | 0x0057,
	BayerRG12p = PX_MONO | PX_BIT_CNT(12) | 0x0059,
	BayerGB12p = PX_MONO | PX_BIT_CNT(12) | 0x0055,
	BayerBG12p = PX_MONO | PX_BIT_CNT(12) | 0x0053,
	BayerGR16 = PX_MONO | PX_BIT_CNT(16) | 0x002E,
	BayerRG16 = PX_MONO | PX_BIT_CNT(16) | 0x002F,
	BayerGB16 = PX_MONO | PX_BIT_CNT(16) | 0x0030,
	BayerBG16 = PX_MONO | PX_BIT_CNT(16) | 0x0031
	// RGB12V1packed = PX_COLOR | PX_BIT_CNT(36) | 0x0034,
	// Double = PX_CUSTOM | PX_MONO | PX_BIT_CNT(64) | 0x100,
	// Confidence8 = PX_MONO | PX_BIT_CNT(8) | 0x00C6,
	// Confidence16 = PX_MONO | PX_BIT_CNT(16) | 0x00C7,
	// Coord3D_C8 = PX_MONO | PX_BIT_CNT(8) | 0x00B1,
	// Coord3D_C16 = PX_MONO | PX_BIT_CNT(16) | 0x00B8,
	// Coord3D_ABC32f = PX_COLOR | PX_BIT_CNT(96) | 0x00C0,
	// Data8 = PX_MONO | PX_BIT_CNT(8) | 0x0116,
	// Data8s = PX_MONO | PX_BIT_CNT(8) | 0x0117,
	// Data16 = PX_MONO | PX_BIT_CNT(16) | 0x0118,
	// Data16s = PX_MONO | PX_BIT_CNT(16) | 0x0119,
	// Data32 = PX_MONO | PX_BIT_CNT(32) | 0x011A,
	// Data32s = PX_MONO | PX_BIT_CNT(32) | 0x011B,
	// Data64 = PX_MONO | PX_BIT_CNT(64) | 0x011D,
	// Data64s = PX_MONO | PX_BIT_CNT(64) | 0x011E,
	// Data32f = PX_MONO | PX_BIT_CNT(32) | 0x011C,
	// Data64f = PX_MONO | PX_BIT_CNT(64) | 0x011F,
};

// A table relating various (foreign) pixel types to info needed to convert
// the image into a standard OpenCV format.
extern const std::array<std::pair<PxType, ConversionInfo>, 49>
	ConversionInfoTable;

// Get color conversion info for a pixel type, if it's supported.
std::optional<ConversionInfo> getConversionInfo(PxType typ);

}  // namespace beholder

#undef PX_MONO
#undef PX_COLOR
#undef PX_CUSTOM
#undef PX_BIT_CNT

#endif	// BEHOLDER_IMAGE_CONVERSION_INFO_H
