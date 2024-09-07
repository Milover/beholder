/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

TODO
	Implement this properly since it's actually super-duper useful.

\*---------------------------------------------------------------------------*/

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

#include "Processor.h"
#include "Ops.h"
#include "Utility.h"

#include "Test.h"

// * * * * * * * * * * * * * * * * Constants * * * * * * * * * * * * * * * * //

namespace beholder
{

// the internal/neural directory
const std::filesystem::path videoDir
{
	"/home/philipp/workspace/transfer"
};

const std::filesystem::path outdir
{
	"/home/philipp/workspace/transfer/frames"
};

} // end namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace fs = std::filesystem;

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{

	int count {-1};
	cv::Mat frame;
	fs::path outpath;

	// chdir to the go project internal/neural directory to make our life easier
	for (const auto& entry : fs::directory_iterator(beholder::videoDir))
	{
		if (!fs::is_regular_file(entry.path()))
		{
			continue;
		}
		cv::VideoCapture cap {entry.path().string()};

		if (!cap.isOpened())
		{
			std::cerr << "could not open video file\n";
			return 1;
		}
		std::cout << "video: " << entry.path().string() << '\n';

		for (int i {0}; i < cap.get(cv::CAP_PROP_FRAME_COUNT); ++i)
		{
			++count;

			cap.set(cv::CAP_PROP_POS_FRAMES, i);

			if (!cap.read(frame))
			{
				std::cerr << "could not read frame\n";
				return 1;
			}

			std::vector<int> flags
			{
				cv::IMWRITE_PNG_COMPRESSION, 0,				// lowest compression level
				cv::IMWRITE_JPEG2000_COMPRESSION_X1000, 0	// lowest compression level
			};

			std::stringstream ss;
			ss << "frame_"
			   << std::setw(7) << std::setfill('0') << count
			   << ".jpeg";
			outpath = beholder::outdir / ss.str();

			std::cout << "writing: " << outpath.string() << '\n';
			cv::imwrite(outpath.string(), frame, flags);
		}
	}
	return 0;
}

// ************************************************************************* //
