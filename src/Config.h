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

#include <opencv2/core/types.hpp>

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
