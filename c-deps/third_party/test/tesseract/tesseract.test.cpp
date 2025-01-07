// Copyright © 2024 P. Milovic
// SPDX-License-Identifier: MIT

// Compile and basic operation test for the Tesseract library.

#include <gtest/gtest.h>
#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <source_location>
#include <sstream>
#include <string>

namespace fs = std::filesystem;
using TBA = tesseract::TessBaseAPI;

// Run OCR on a local image and compare read text to a reference, also read
// from a local file.
TEST(tesseract, SimpleOCR) {  // NOLINT
	// set up the testdata directory path
	const fs::path data{
		fs::absolute(std::source_location::current().file_name())
			.parent_path() /
		"testdata"};

	// read the expected text from a file
	std::ifstream ifs{fs::path{data / "phototest.tif.txt"}};
	ASSERT_TRUE(ifs.is_open());

	std::stringstream buffer;
	buffer << ifs.rdbuf();
	const std::string expected{buffer.str()};
	ASSERT_NE(expected.size(), 0);

	auto tDel = [](TBA* ptr) {
		ptr->End();
		delete ptr;	 // NOLINT
	};
	std::unique_ptr<TBA, decltype(tDel)> api{new TBA{}, tDel};
	ASSERT_NE(api, nullptr);
	ASSERT_EQ(api->Init(data.c_str(), "eng"), 0);

	// Open input image with leptonica library
	auto pDel = [](Pix* ptr) { pixDestroy(&ptr); };
	const std::unique_ptr<Pix, decltype(pDel)> image{
		pixRead(fs::path{data / "phototest.tif"}.c_str()), pDel};
	ASSERT_NE(image, nullptr);

	api->SetImage(image.get());

	// Get OCR result
	std::unique_ptr<char[]> result{};  // NOLINT
	result.reset(api->GetUTF8Text());
	EXPECT_STREQ(result.get(), expected.c_str());
}
