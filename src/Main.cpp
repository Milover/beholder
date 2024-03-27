/*---------------------------------------------------------------------------*\

	ocr - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

\*---------------------------------------------------------------------------*/

#include <iostream>
#include <memory>
#include <string>

#include <tesseract/baseapi.h>
#include <allheaders.h>

#include "Main.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

namespace ocr {

	// FIXME: hardcoded traineddata dir
	static const char* const trainedDataDir
	{
		"/Users/philipp/Documents/c++/libs/tesseract/tessdata/fast"
	};

	// FIXME: hardcoded test image
	static const char* const testPhoto
	{
		"/Users/philipp/Documents/c++/projects/ocr/test/testdata/simple_12pt.tif"
	};
}

using tAPI = tesseract::TessBaseAPI;

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	auto tAPIDel = [](tAPI* p) {
		p->End();
		delete p;
	};
	auto api = std::unique_ptr<tAPI, decltype(tAPIDel)>(new tAPI{}, tAPIDel);

	// Initialize tesseract-ocr with English, without specifying tessdata path
	if (api->Init(ocr::trainedDataDir, "eng")) {
		std::cerr << "Could not initialize tesseract.\n";
		return 1;
	}

	// Open input image with leptonica library
	auto pixDel = [](Pix* p) { pixDestroy(&p); };
	auto image = std::shared_ptr<Pix>(pixRead(ocr::testPhoto), pixDel);
	api->SetImage(image.get());

	// Get OCR result
	std::string outText = std::string(api->GetUTF8Text());
	std::cout << "OCR output:\n" << outText;

	return 0;
}


// ************************************************************************* //
