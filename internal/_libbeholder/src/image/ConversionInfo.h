/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A helper class for holding information needed for receiving/converting
	camera acquisition results.

\*---------------------------------------------------------------------------*/

#ifndef BEHOLDER_CONVERSION_INFO_H
#define BEHOLDER_CONVERSION_INFO_H

#include <array>
#include <utility>

#include <opencv2/imgproc.hpp>

#include <pylon/PixelType.h>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{

/*---------------------------------------------------------------------------*\
                    Class ConversionInfo Declaration
\*---------------------------------------------------------------------------*/

class ConversionInfo
{
public:

	// Public data

		//- The CvMat type of the the (foreign) image buffer
		const int inputType;
		//- The number of channels of the output image
		const int outChannels;
		//- The cv color conversion code
		const int colorConvCode;
};

// * * * * * * * * * * * * * * Global Variables  * * * * * * * * * * * * * * //

//- A table relating various (foreign) pixel types to info needed to convert
//	the image into a standard OpenCV format.
inline static constexpr std::array<std::pair<Pylon::EPixelType, ConversionInfo>, 49>
ConversionInfoTable
{{
//	{Pylon::PixelType_Undefined,						{CV_8UC1,	CV_8UC1,	-1}},
	{Pylon::PixelType_Mono1packed,						{CV_8UC1,	1,	-1}},
	{Pylon::PixelType_Mono2packed,						{CV_8UC1,	1,	-1}},
	{Pylon::PixelType_Mono4packed,						{CV_8UC1,	1,	-1}},
	{Pylon::PixelType_Mono8,							{CV_8UC1,	1,	-1}},
	{Pylon::PixelType_Mono8signed,						{CV_8SC1,	1,	-1}},
	{Pylon::PixelType_Mono10,							{CV_16UC1,	1,	-1}},
	{Pylon::PixelType_Mono10packed,						{CV_16UC1,	1,	-1}},
	{Pylon::PixelType_Mono10p,							{CV_16UC1,	1,	-1}},
	{Pylon::PixelType_Mono12,							{CV_16UC1,	1,	-1}},
	{Pylon::PixelType_Mono12packed,						{CV_16UC1,	1,	-1}},
	{Pylon::PixelType_Mono12p,							{CV_16UC1,	1,	-1}},
	{Pylon::PixelType_Mono16,							{CV_16UC1,	1,	-1}},
	{Pylon::PixelType_BayerGR8,							{CV_8UC1,	3,	cv::COLOR_BayerGRBG2BGR}},
	{Pylon::PixelType_BayerRG8,							{CV_8UC1,	3,	cv::COLOR_BayerRGGB2BGR}},
	{Pylon::PixelType_BayerGB8,							{CV_8UC1,	3,	cv::COLOR_BayerGBRG2BGR}},
	{Pylon::PixelType_BayerBG8,							{CV_8UC1,	3,	cv::COLOR_BayerBGGR2BGR}},
	{Pylon::PixelType_BayerGR10,						{CV_16UC1,	3,	cv::COLOR_BayerGRBG2BGR}},
	{Pylon::PixelType_BayerRG10,						{CV_16UC1,	3,	cv::COLOR_BayerRGGB2BGR}},
	{Pylon::PixelType_BayerGB10,						{CV_16UC1,	3,	cv::COLOR_BayerGBRG2BGR}},
	{Pylon::PixelType_BayerBG10,						{CV_16UC1,	3,	cv::COLOR_BayerGBRG2BGR}},
	{Pylon::PixelType_BayerGR12,						{CV_16UC1,	3,	cv::COLOR_BayerGRBG2BGR}},
	{Pylon::PixelType_BayerRG12,						{CV_16UC1,	3,	cv::COLOR_BayerRGGB2BGR}},
	{Pylon::PixelType_BayerGB12,						{CV_16UC1,	3,	cv::COLOR_BayerGBRG2BGR}},
	{Pylon::PixelType_BayerBG12,						{CV_16UC1,	3,	cv::COLOR_BayerGBRG2BGR}},
	{Pylon::PixelType_RGB8packed,						{CV_8UC3,	3,	cv::COLOR_RGB2BGR}},
	{Pylon::PixelType_BGR8packed,						{CV_8UC3,	3,	-1}},
	{Pylon::PixelType_RGBA8packed,						{CV_8UC4,	3,	cv::COLOR_RGBA2BGR}},
	{Pylon::PixelType_BGRA8packed,						{CV_8UC4,	3,	cv::COLOR_BGRA2BGR}},
	{Pylon::PixelType_RGB10packed,						{CV_16UC3,	3,	cv::COLOR_RGB2BGR}},
	{Pylon::PixelType_BGR10packed,						{CV_16UC3,	3,	-1}},
	{Pylon::PixelType_RGB12packed,						{CV_16UC3,	3,	cv::COLOR_RGB2BGR}},
	{Pylon::PixelType_BGR12packed,						{CV_16UC3,	3,	-1}},
	{Pylon::PixelType_RGB16packed,						{CV_16UC3,	3,	cv::COLOR_RGB2BGR}},
//	{Pylon::PixelType_BGR10V1packed,					{CV_8UC1,	CV_8UC1,	-1}},
//	{Pylon::PixelType_BGR10V2packed,					{CV_8UC1,	CV_8UC1,	-1}},
//	{Pylon::PixelType_YUV411packed,						{CV_8UC1,	CV_8UC1,	-1}},
//	{Pylon::PixelType_YUV422packed,						{CV_8UC1,	CV_8UC1,	-1}},
//	{Pylon::PixelType_YUV444packed,						{CV_8UC1,	CV_8UC1,	-1}},
//	{Pylon::PixelType_RGB8planar,						{CV_8UC1,	CV_8UC1,	-1}},
//	{Pylon::PixelType_RGB10planar,						{CV_8UC1,	CV_8UC1,	-1}},
//	{Pylon::PixelType_RGB12planar,						{CV_8UC1,	CV_8UC1,	-1}},
//	{Pylon::PixelType_RGB16planar,						{CV_8UC1,	CV_8UC1,	-1}},
//	{Pylon::PixelType_YUV422_YUYV_Packed,				{CV_8UC1,	CV_8UC1,	-1}},
//	{Pylon::PixelType_YUV444planar,						{CV_8UC1,	CV_8UC1,	-1}},
//	{Pylon::PixelType_YUV422planar,						{CV_8UC1,	CV_8UC1,	-1}},
//	{Pylon::PixelType_YUV420planar,						{CV_8UC1,	CV_8UC1,	-1}},
//	{Pylon::PixelType_YCbCr420_8_YY_CbCr_Semiplanar,	{CV_8UC1,	CV_8UC1,	-1}},
//	{Pylon::PixelType_YCbCr422_8_YY_CbCr_Semiplanar,	{CV_8UC1,	CV_8UC1,	-1}},
	{Pylon::PixelType_BayerGR12Packed,					{CV_16UC1,	3,	cv::COLOR_BayerGRBG2BGR}},
	{Pylon::PixelType_BayerRG12Packed,					{CV_16UC1,	3,	cv::COLOR_BayerRGGB2BGR}},
	{Pylon::PixelType_BayerGB12Packed,					{CV_16UC1,	3,	cv::COLOR_BayerGBRG2BGR}},
	{Pylon::PixelType_BayerBG12Packed,					{CV_16UC1,	3,	cv::COLOR_BayerGBRG2BGR}},
	{Pylon::PixelType_BayerGR10p,						{CV_16UC1,	3,	cv::COLOR_BayerGRBG2BGR}},
	{Pylon::PixelType_BayerRG10p,						{CV_16UC1,	3,	cv::COLOR_BayerRGGB2BGR}},
	{Pylon::PixelType_BayerGB10p,						{CV_16UC1,	3,	cv::COLOR_BayerGBRG2BGR}},
	{Pylon::PixelType_BayerBG10p,						{CV_16UC1,	3,	cv::COLOR_BayerGBRG2BGR}},
	{Pylon::PixelType_BayerGR12p,						{CV_16UC1,	3,	cv::COLOR_BayerGRBG2BGR}},
	{Pylon::PixelType_BayerRG12p,						{CV_16UC1,	3,	cv::COLOR_BayerRGGB2BGR}},
	{Pylon::PixelType_BayerGB12p,						{CV_16UC1,	3,	cv::COLOR_BayerGBRG2BGR}},
	{Pylon::PixelType_BayerBG12p,						{CV_16UC1,	3,	cv::COLOR_BayerGBRG2BGR}},
	{Pylon::PixelType_BayerGR16,						{CV_16UC1,	3,	cv::COLOR_BayerGRBG2BGR}},
	{Pylon::PixelType_BayerRG16,						{CV_16UC1,	3,	cv::COLOR_BayerRGGB2BGR}},
	{Pylon::PixelType_BayerGB16,						{CV_16UC1,	3,	cv::COLOR_BayerGBRG2BGR}},
	{Pylon::PixelType_BayerBG16,						{CV_16UC1,	3,	cv::COLOR_BayerGBRG2BGR}}
//	{Pylon::PixelType_RGB12V1packed,					{CV_8UC1,	CV_8UC1,	-1}},
//	{Pylon::PixelType_Double,							{CV_8UC1,	CV_8UC1,	-1}},
//	{Pylon::PixelType_Confidence8,						{CV_8UC1,	CV_8UC1,	-1}},
//	{Pylon::PixelType_Confidence16,						{CV_8UC1,	CV_8UC1,	-1}},
//	{Pylon::PixelType_Coord3D_C8,						{CV_8UC1,	CV_8UC1,	-1}},
//	{Pylon::PixelType_Coord3D_C16,						{CV_8UC1,	CV_8UC1,	-1}},
//	{Pylon::PixelType_Coord3D_ABC32f,					{CV_8UC1,	CV_8UC1,	-1}},
//	{Pylon::PixelType_Data8,							{CV_8UC1,	CV_8UC1,	-1}},
//	{Pylon::PixelType_Data8s,							{CV_8UC1,	CV_8UC1,	-1}},
//	{Pylon::PixelType_Data16,							{CV_8UC1,	CV_8UC1,	-1}},
//	{Pylon::PixelType_Data16s,							{CV_8UC1,	CV_8UC1,	-1}},
//	{Pylon::PixelType_Data32,							{CV_8UC1,	CV_8UC1,	-1}},
//	{Pylon::PixelType_Data32s,							{CV_8UC1,	CV_8UC1,	-1}},
//	{Pylon::PixelType_Data64,							{CV_8UC1,	CV_8UC1,	-1}},
//	{Pylon::PixelType_Data64s,							{CV_8UC1,	CV_8UC1,	-1}},
//	{Pylon::PixelType_Data32f,							{CV_8UC1,	CV_8UC1,	-1}},
//	{Pylon::PixelType_Data64f,							{CV_8UC1,	CV_8UC1,	-1}},
}};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
