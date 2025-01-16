// beholder - Copyright © 2024 Philipp Milovic
//
// SPDX-License-Identifier: Apache-2.0

#include "beholder/image/ConversionInfo.h"

#include <array>
#include <opencv2/imgproc.hpp>
#include <optional>
#include <utility>

namespace beholder {

const std::array<std::pair<PxType, ConversionInfo>, 49> ConversionInfoTable{{
	// {PxType::Undefined, {CV_8UC1, 1, -1}},
	{PxType::Mono1packed, {CV_8UC1, 1, -1}},
	{PxType::Mono2packed, {CV_8UC1, 1, -1}},
	{PxType::Mono4packed, {CV_8UC1, 1, -1}},
	{PxType::Mono8, {CV_8UC1, 1, -1}},
	{PxType::Mono8signed, {CV_8SC1, 1, -1}},
	{PxType::Mono10, {CV_16UC1, 1, -1}},
	{PxType::Mono10packed, {CV_16UC1, 1, -1}},
	{PxType::Mono10p, {CV_16UC1, 1, -1}},
	{PxType::Mono12, {CV_16UC1, 1, -1}},
	{PxType::Mono12packed, {CV_16UC1, 1, -1}},
	{PxType::Mono12p, {CV_16UC1, 1, -1}},
	{PxType::Mono16, {CV_16UC1, 1, -1}},
	{PxType::BayerGR8, {CV_8UC1, 3, cv::COLOR_BayerGRBG2BGR}},
	{PxType::BayerRG8, {CV_8UC1, 3, cv::COLOR_BayerRGGB2BGR}},
	{PxType::BayerGB8, {CV_8UC1, 3, cv::COLOR_BayerGBRG2BGR}},
	{PxType::BayerBG8, {CV_8UC1, 3, cv::COLOR_BayerBGGR2BGR}},
	{PxType::BayerGR10, {CV_16UC1, 3, cv::COLOR_BayerGRBG2BGR}},
	{PxType::BayerRG10, {CV_16UC1, 3, cv::COLOR_BayerRGGB2BGR}},
	{PxType::BayerGB10, {CV_16UC1, 3, cv::COLOR_BayerGBRG2BGR}},
	{PxType::BayerBG10, {CV_16UC1, 3, cv::COLOR_BayerGBRG2BGR}},
	{PxType::BayerGR12, {CV_16UC1, 3, cv::COLOR_BayerGRBG2BGR}},
	{PxType::BayerRG12, {CV_16UC1, 3, cv::COLOR_BayerRGGB2BGR}},
	{PxType::BayerGB12, {CV_16UC1, 3, cv::COLOR_BayerGBRG2BGR}},
	{PxType::BayerBG12, {CV_16UC1, 3, cv::COLOR_BayerGBRG2BGR}},
	{PxType::RGB8packed, {CV_8UC3, 3, cv::COLOR_RGB2BGR}},
	{PxType::BGR8packed, {CV_8UC3, 3, -1}},
	{PxType::RGBA8packed, {CV_8UC4, 3, cv::COLOR_RGBA2BGR}},
	{PxType::BGRA8packed, {CV_8UC4, 3, cv::COLOR_BGRA2BGR}},
	{PxType::RGB10packed, {CV_16UC3, 3, cv::COLOR_RGB2BGR}},
	{PxType::BGR10packed, {CV_16UC3, 3, -1}},
	{PxType::RGB12packed, {CV_16UC3, 3, cv::COLOR_RGB2BGR}},
	{PxType::BGR12packed, {CV_16UC3, 3, -1}},
	{PxType::RGB16packed, {CV_16UC3, 3, cv::COLOR_RGB2BGR}},
	// {PxType::BGR10V1packed, {CV_8UC1, CV_8UC1, -1}},
	// {PxType::BGR10V2packed, {CV_8UC1, CV_8UC1, -1}},
	// {PxType::YUV411packed, {CV_8UC1, CV_8UC1, -1}},
	// {PxType::YUV422packed, {CV_8UC1, CV_8UC1, -1}},
	// {PxType::YUV444packed, {CV_8UC1, CV_8UC1, -1}},
	// {PxType::RGB8planar, {CV_8UC1, CV_8UC1, -1}},
	// {PxType::RGB10planar, {CV_8UC1, CV_8UC1, -1}},
	// {PxType::RGB12planar, {CV_8UC1, CV_8UC1, -1}},
	// {PxType::RGB16planar, {CV_8UC1, CV_8UC1, -1}},
	// {PxType::YUV422_YUYV_Packed, {CV_8UC1, CV_8UC1, -1}},
	// {PxType::YUV444planar, {CV_8UC1, CV_8UC1, -1}},
	// {PxType::YUV422planar, {CV_8UC1, CV_8UC1, -1}},
	// {PxType::YUV420planar, {CV_8UC1, CV_8UC1, -1}},
	// {PxType::YCbCr420_8_YY_CbCr_Semiplanar, {CV_8UC1, CV_8UC1, -1}},
	// {PxType::YCbCr422_8_YY_CbCr_Semiplanar, {CV_8UC1, CV_8UC1, -1}},
	{PxType::BayerGR12Packed, {CV_16UC1, 3, cv::COLOR_BayerGRBG2BGR}},
	{PxType::BayerRG12Packed, {CV_16UC1, 3, cv::COLOR_BayerRGGB2BGR}},
	{PxType::BayerGB12Packed, {CV_16UC1, 3, cv::COLOR_BayerGBRG2BGR}},
	{PxType::BayerBG12Packed, {CV_16UC1, 3, cv::COLOR_BayerGBRG2BGR}},
	{PxType::BayerGR10p, {CV_16UC1, 3, cv::COLOR_BayerGRBG2BGR}},
	{PxType::BayerRG10p, {CV_16UC1, 3, cv::COLOR_BayerRGGB2BGR}},
	{PxType::BayerGB10p, {CV_16UC1, 3, cv::COLOR_BayerGBRG2BGR}},
	{PxType::BayerBG10p, {CV_16UC1, 3, cv::COLOR_BayerGBRG2BGR}},
	{PxType::BayerGR12p, {CV_16UC1, 3, cv::COLOR_BayerGRBG2BGR}},
	{PxType::BayerRG12p, {CV_16UC1, 3, cv::COLOR_BayerRGGB2BGR}},
	{PxType::BayerGB12p, {CV_16UC1, 3, cv::COLOR_BayerGBRG2BGR}},
	{PxType::BayerBG12p, {CV_16UC1, 3, cv::COLOR_BayerGBRG2BGR}},
	{PxType::BayerGR16, {CV_16UC1, 3, cv::COLOR_BayerGRBG2BGR}},
	{PxType::BayerRG16, {CV_16UC1, 3, cv::COLOR_BayerRGGB2BGR}},
	{PxType::BayerGB16, {CV_16UC1, 3, cv::COLOR_BayerGBRG2BGR}},
	{PxType::BayerBG16, {CV_16UC1, 3, cv::COLOR_BayerGBRG2BGR}}
	// {PxType::RGB12V1packed, {CV_8UC1, CV_8UC1, -1}},
	// {PxType::Double, {CV_8UC1, CV_8UC1, -1}},
	// {PxType::Confidence8, {CV_8UC1, CV_8UC1, -1}},
	// {PxType::Confidence16, {CV_8UC1, CV_8UC1, -1}},
	// {PxType::Coord3D_C8, {CV_8UC1, CV_8UC1, -1}},
	// {PxType::Coord3D_C16, {CV_8UC1, CV_8UC1, -1}},
	// {PxType::Coord3D_ABC32f, {CV_8UC1, CV_8UC1, -1}},
	// {PxType::Data8, {CV_8UC1, CV_8UC1, -1}},
	// {PxType::Data8s, {CV_8UC1, CV_8UC1, -1}},
	// {PxType::Data16, {CV_8UC1, CV_8UC1, -1}},
	// {PxType::Data16s, {CV_8UC1, CV_8UC1, -1}},
	// {PxType::Data32, {CV_8UC1, CV_8UC1, -1}},
	// {PxType::Data32s, {CV_8UC1, CV_8UC1, -1}},
	// {PxType::Data64, {CV_8UC1, CV_8UC1, -1}},
	// {PxType::Data64s, {CV_8UC1, CV_8UC1, -1}},
	// {PxType::Data32f, {CV_8UC1, CV_8UC1, -1}},
	// {PxType::Data64f, {CV_8UC1, CV_8UC1, -1}}
}};

std::optional<ConversionInfo> getConversionInfo(PxType typ) {
	const auto* found{
		std::find_if(ConversionInfoTable.begin(), ConversionInfoTable.end(),
					 [typ](const auto& p) -> bool { return p.first == typ; })};
	if (found == ConversionInfoTable.end()) {
		return std::nullopt;
	}
	return found->second;
}

}  // namespace beholder
