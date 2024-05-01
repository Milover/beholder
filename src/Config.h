/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A configuration data class.

SourceFiles
	Config.cpp

\*---------------------------------------------------------------------------*/

#ifndef OCR_CONFIG_H
#define OCR_CONFIG_H

#include <array>
#include <string>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace ocr
{

/*---------------------------------------------------------------------------*\
                          Class Config Declaration
\*---------------------------------------------------------------------------*/

class Config
{
private:

	// Private data

public:

	// Public data

		//- Paths of Tesseract configuration files
		std::vector<std::string> configPaths;
		//- Path to Tesseract model file directory
		std::string modelPath;
		//- Name of the Tesseract model
		std::string model;
		//- The RGBA color of the text box drawn during text detection
		std::array<double, 4> textBoxColor;
		//- The line thickness of the text box drawn during text detection
		int textBoxThickness;

	// Constructors

	//- Destructor

	// Member functions

		//- Parse data from a JSON string
		//
		//	For details see: https://json.nlohmann.me/api/basic_json/parse/
		template<typename T>
		bool parse(T&& t);

	// Member operators

};

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

//- Allow marshalling to JSON
void to_json(nlohmann::json& j, const Config& c);

//- Allow unmarshalling from JSON
void from_json(const nlohmann::json& j, Config& c);

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

template<typename T>
bool Config::parse(T&& t)
{
	try
	{
		auto j = nlohmann::json::parse(std::forward<T>(t));
		*this = j.template get<Config>();
	}
	catch(...)
	{
		return false;
	}
	return true;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace ocr

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
