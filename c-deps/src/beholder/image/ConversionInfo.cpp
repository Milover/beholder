/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <array>
#include <optional>
#include <utility>

#include <opencv2/imgproc.hpp>

#include "image/ConversionInfo.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{

// * * * * * * * * * * * * * * Global Variables  * * * * * * * * * * * * * * //

const std::array<std::pair<PxType, ConversionInfo>, 49> ConversionInfoTable
{{
//	{PxType::Undefined,                     {CV_8UC1,	1,	-1}},                       // Undefined
	{PxType::Mono1packed,                   {CV_8UC1,	1,	-1}},                       // Mono1packed
	{PxType::Mono2packed,                   {CV_8UC1,	1,	-1}},                       // Mono2packed
	{PxType::Mono4packed,                   {CV_8UC1,	1,	-1}},                       // Mono4packed
	{PxType::Mono8,                         {CV_8UC1,	1,	-1}},                       // Mono8
	{PxType::Mono8signed,                   {CV_8SC1,	1,	-1}},                       // Mono8signed
	{PxType::Mono10,                        {CV_16UC1,	1,	-1}},                       // Mono10
	{PxType::Mono10packed,                  {CV_16UC1,	1,	-1}},                       // Mono10packed
	{PxType::Mono10p,                       {CV_16UC1,	1,	-1}},                       // Mono10p
	{PxType::Mono12,                        {CV_16UC1,	1,	-1}},                       // Mono12
	{PxType::Mono12packed,                  {CV_16UC1,	1,	-1}},                       // Mono12packed
	{PxType::Mono12p,                       {CV_16UC1,	1,	-1}},                       // Mono12p
	{PxType::Mono16,                        {CV_16UC1,	1,	-1}},                       // Mono16
	{PxType::BayerGR8,                      {CV_8UC1,	3,	cv::COLOR_BayerGRBG2BGR}},  // BayerGR8
	{PxType::BayerRG8,                      {CV_8UC1,	3,	cv::COLOR_BayerRGGB2BGR}},  // BayerRG8
	{PxType::BayerGB8,                      {CV_8UC1,	3,	cv::COLOR_BayerGBRG2BGR}},  // BayerGB8
	{PxType::BayerBG8,                      {CV_8UC1,	3,	cv::COLOR_BayerBGGR2BGR}},  // BayerBG8
	{PxType::BayerGR10,                     {CV_16UC1,	3,	cv::COLOR_BayerGRBG2BGR}},  // BayerGR10
	{PxType::BayerRG10,                     {CV_16UC1,	3,	cv::COLOR_BayerRGGB2BGR}},  // BayerRG10
	{PxType::BayerGB10,                     {CV_16UC1,	3,	cv::COLOR_BayerGBRG2BGR}},  // BayerGB10
	{PxType::BayerBG10,                     {CV_16UC1,	3,	cv::COLOR_BayerGBRG2BGR}},  // BayerBG10
	{PxType::BayerGR12,                     {CV_16UC1,	3,	cv::COLOR_BayerGRBG2BGR}},  // BayerGR12
	{PxType::BayerRG12,                     {CV_16UC1,	3,	cv::COLOR_BayerRGGB2BGR}},  // BayerRG12
	{PxType::BayerGB12,                     {CV_16UC1,	3,	cv::COLOR_BayerGBRG2BGR}},  // BayerGB12
	{PxType::BayerBG12,                     {CV_16UC1,	3,	cv::COLOR_BayerGBRG2BGR}},  // BayerBG12
	{PxType::RGB8packed,                    {CV_8UC3,	3,	cv::COLOR_RGB2BGR}},        // RGB8packed
	{PxType::BGR8packed,                    {CV_8UC3,	3,	-1}},                       // BGR8packed
	{PxType::RGBA8packed,                   {CV_8UC4,	3,	cv::COLOR_RGBA2BGR}},       // RGBA8packed
	{PxType::BGRA8packed,                   {CV_8UC4,	3,	cv::COLOR_BGRA2BGR}},       // BGRA8packed
	{PxType::RGB10packed,                   {CV_16UC3,	3,	cv::COLOR_RGB2BGR}},        // RGB10packed
	{PxType::BGR10packed,                   {CV_16UC3,	3,	-1}},                       // BGR10packed
	{PxType::RGB12packed,                   {CV_16UC3,	3,	cv::COLOR_RGB2BGR}},        // RGB12packed
	{PxType::BGR12packed,                   {CV_16UC3,	3,	-1}},                       // BGR12packed
	{PxType::RGB16packed,                   {CV_16UC3,	3,	cv::COLOR_RGB2BGR}},        // RGB16packed
//	{PxType::BGR10V1packed,                 {CV_8UC1,	CV_8UC1,	-1}},               // BGR10V1packed
//	{PxType::BGR10V2packed,                 {CV_8UC1,	CV_8UC1,	-1}},               // BGR10V2packed
//	{PxType::YUV411packed,                  {CV_8UC1,	CV_8UC1,	-1}},               // YUV411packed
//	{PxType::YUV422packed,                  {CV_8UC1,	CV_8UC1,	-1}},               // YUV422packed
//	{PxType::YUV444packed,                  {CV_8UC1,	CV_8UC1,	-1}},               // YUV444packed
//	{PxType::RGB8planar,                    {CV_8UC1,	CV_8UC1,	-1}},               // RGB8planar
//	{PxType::RGB10planar,                   {CV_8UC1,	CV_8UC1,	-1}},               // RGB10planar
//	{PxType::RGB12planar,                   {CV_8UC1,	CV_8UC1,	-1}},               // RGB12planar
//	{PxType::RGB16planar,                   {CV_8UC1,	CV_8UC1,	-1}},               // RGB16planar
//	{PxType::YUV422_YUYV_Packed,            {CV_8UC1,	CV_8UC1,	-1}},               // YUV422_YUYV_Packed
//	{PxType::YUV444planar,                  {CV_8UC1,	CV_8UC1,	-1}},               // YUV444planar
//	{PxType::YUV422planar,                  {CV_8UC1,	CV_8UC1,	-1}},               // YUV422planar
//	{PxType::YUV420planar,                  {CV_8UC1,	CV_8UC1,	-1}},               // YUV420planar
//	{PxType::YCbCr420_8_YY_CbCr_Semiplanar, {CV_8UC1,	CV_8UC1,	-1}},               // YCbCr420_8_YY_CbCr_Semiplanar
//	{PxType::YCbCr422_8_YY_CbCr_Semiplanar, {CV_8UC1,	CV_8UC1,	-1}},               // YCbCr422_8_YY_CbCr_Semiplanar
	{PxType::BayerGR12Packed,               {CV_16UC1,	3,	cv::COLOR_BayerGRBG2BGR}},  // BayerGR12Packed
	{PxType::BayerRG12Packed,               {CV_16UC1,	3,	cv::COLOR_BayerRGGB2BGR}},  // BayerRG12Packed
	{PxType::BayerGB12Packed,               {CV_16UC1,	3,	cv::COLOR_BayerGBRG2BGR}},  // BayerGB12Packed
	{PxType::BayerBG12Packed,               {CV_16UC1,	3,	cv::COLOR_BayerGBRG2BGR}},  // BayerBG12Packed
	{PxType::BayerGR10p,                    {CV_16UC1,	3,	cv::COLOR_BayerGRBG2BGR}},  // BayerGR10p
	{PxType::BayerRG10p,                    {CV_16UC1,	3,	cv::COLOR_BayerRGGB2BGR}},  // BayerRG10p
	{PxType::BayerGB10p,                    {CV_16UC1,	3,	cv::COLOR_BayerGBRG2BGR}},  // BayerGB10p
	{PxType::BayerBG10p,                    {CV_16UC1,	3,	cv::COLOR_BayerGBRG2BGR}},  // BayerBG10p
	{PxType::BayerGR12p,                    {CV_16UC1,	3,	cv::COLOR_BayerGRBG2BGR}},  // BayerGR12p
	{PxType::BayerRG12p,                    {CV_16UC1,	3,	cv::COLOR_BayerRGGB2BGR}},  // BayerRG12p
	{PxType::BayerGB12p,                    {CV_16UC1,	3,	cv::COLOR_BayerGBRG2BGR}},  // BayerGB12p
	{PxType::BayerBG12p,                    {CV_16UC1,	3,	cv::COLOR_BayerGBRG2BGR}},  // BayerBG12p
	{PxType::BayerGR16,                     {CV_16UC1,	3,	cv::COLOR_BayerGRBG2BGR}},  // BayerGR16
	{PxType::BayerRG16,                     {CV_16UC1,	3,	cv::COLOR_BayerRGGB2BGR}},  // BayerRG16
	{PxType::BayerGB16,                     {CV_16UC1,	3,	cv::COLOR_BayerGBRG2BGR}},  // BayerGB16
	{PxType::BayerBG16,                     {CV_16UC1,	3,	cv::COLOR_BayerGBRG2BGR}}   // BayerBG16
//	{PxType::RGB12V1packed,                 {CV_8UC1,	CV_8UC1,	-1}},               // RGB12V1packed
//	{PxType::Double,                        {CV_8UC1,	CV_8UC1,	-1}},               // Double
//	{PxType::Confidence8,                   {CV_8UC1,	CV_8UC1,	-1}},               // Confidence8
//	{PxType::Confidence16,                  {CV_8UC1,	CV_8UC1,	-1}},               // Confidence16
//	{PxType::Coord3D_C8,                    {CV_8UC1,	CV_8UC1,	-1}},               // Coord3D_C8
//	{PxType::Coord3D_C16,                   {CV_8UC1,	CV_8UC1,	-1}},               // Coord3D_C16
//	{PxType::Coord3D_ABC32f,                {CV_8UC1,	CV_8UC1,	-1}},               // Coord3D_ABC32f
//	{PxType::Data8,                         {CV_8UC1,	CV_8UC1,	-1}},               // Data8
//	{PxType::Data8s,                        {CV_8UC1,	CV_8UC1,	-1}},               // Data8s
//	{PxType::Data16,                        {CV_8UC1,	CV_8UC1,	-1}},               // Data16
//	{PxType::Data16s,                       {CV_8UC1,	CV_8UC1,	-1}},               // Data16s
//	{PxType::Data32,                        {CV_8UC1,	CV_8UC1,	-1}},               // Data32
//	{PxType::Data32s,                       {CV_8UC1,	CV_8UC1,	-1}},               // Data32s
//	{PxType::Data64,                        {CV_8UC1,	CV_8UC1,	-1}},               // Data64
//	{PxType::Data64s,                       {CV_8UC1,	CV_8UC1,	-1}},               // Data64s
//	{PxType::Data32f,                       {CV_8UC1,	CV_8UC1,	-1}},               // Data32f
//	{PxType::Data64f,                       {CV_8UC1,	CV_8UC1,	-1}},               // Data64f
}};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

std::optional<ConversionInfo> getConversionInfo(PxType typ)
{
	auto found
	{
		std::find_if
		(
			ConversionInfoTable.begin(),
			ConversionInfoTable.end(),
			[typ](const auto& p) -> bool { return p.first == typ; }
		)
	};
	if (found == ConversionInfoTable.end())
	{
		return std::nullopt;
	}
	return found->second;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// ************************************************************************* //
