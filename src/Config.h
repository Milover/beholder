/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A C-compatible struct for holding configuration data

\*---------------------------------------------------------------------------*/

#ifndef OCR_CONFIG_H
#define OCR_CONFIG_H

#ifdef __cplusplus
namespace ocr
{
extern "C"
{
#endif

// * * * * * * * * * * * * Struct Config Definition  * * * * * * * * * * * * //

struct Config
{
	char** configPaths;
	int nConfigPaths;

	char* modelPath;
	char* model;

	double textBoxColor[4];
	int textBoxThickness;
};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

inline void ConfigDelete(struct Config* cc)
{
	for (auto i {0}; i < cc->nConfigPaths; ++i)
	{
		delete[] cc->configPaths[i];
	}
	delete cc->configPaths;
	cc->configPaths = nullptr;
	delete cc->modelPath;
	cc->modelPath = nullptr;
	delete cc->model;
	cc->model = nullptr;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef __cplusplus
} // end extern "C"
} // end namespace ocr
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
