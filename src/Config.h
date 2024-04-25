/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A class for holding configuration data.

SourceFiles
	Config.cpp

\*---------------------------------------------------------------------------*/

#ifndef OCR_CONFIG_H
#define OCR_CONFIG_H

#include <string>
#include <vector>

#include <opencv2/opencv.hpp>	// TODO: break this out, we don't need all of it

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace ocr
{

/*---------------------------------------------------------------------------*\
                          Class Config Declaration
\*---------------------------------------------------------------------------*/

class Config
{
public:
	std::vector<std::string> configPaths;
	std::string modelPath;
	std::string model;

	cv::Scalar textBoxColor {0, 0, 0};
	int textBoxThickness {3};
};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace ocr

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
