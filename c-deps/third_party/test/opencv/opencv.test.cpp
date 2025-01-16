// Copyright © 2024 P. Milovic
// SPDX-License-Identifier: Apache-2.0

// Compile and basic operation test for the OpenCV library.

#include <gtest/gtest.h>

#include <filesystem>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <source_location>

namespace fs = std::filesystem;

// Load a 100x100 px purely red (255, 0, 0) image from disc.
TEST(opencv, ImgRead) {	 // NOLINT
	// set up the testdata directory path
	const fs::path data{
		fs::absolute(std::source_location::current().file_name())
			.parent_path() /
		"testdata/red_100x100.png"};

	// read and decode the image
	cv::Mat img{cv::imread(data.string(), cv::IMREAD_COLOR)};
	ASSERT_FALSE(img.empty());
	EXPECT_EQ(img.rows, 100);
	EXPECT_EQ(img.cols, 100);
	EXPECT_EQ(img.channels(), 3);

	// check the color of a pixel
	const cv::Vec3b px{img.at<cv::Vec3b>(50, 50)};
	EXPECT_EQ(px.val[0], 0);
	EXPECT_EQ(px.val[1], 0);
	EXPECT_EQ(px.val[2], 255);
}
