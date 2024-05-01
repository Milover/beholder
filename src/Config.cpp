/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <array>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "Config.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace ocr
{

// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * Helper Functions  * * * * * * * * * * * * * * //

void to_json(nlohmann::json& j, const Config& c)
{
	j = nlohmann::json
	{
		{"config_paths", c.configPaths},
		{"model_path", c.modelPath},
		{"model", c.model},
		{"text_box_color", c.textBoxColor},
		{"text_box_thickness", c.textBoxThickness}
	};
}


void from_json(const nlohmann::json& j, Config& c)
{
	c.configPaths = j.value("config_paths", std::vector<std::string>{});
	c.modelPath = j.value("model_path", "");
	c.model = j.value("model", "");
	c.textBoxColor = j.value("text_box_color", std::array<double, 4>{0.0, 0.0, 0.0, 0.0});
	c.textBoxThickness = j.value("text_box_thickness", 3);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace ocr

// ************************************************************************* //
