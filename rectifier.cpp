#include "rectifier.h"
#include <fstream>
#include <sstream>
#include "rectifier.h"
#include <vector>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>

void parseLine(std::vector<double> &dst, const std::string &line)
{
	using namespace std;
	istringstream iss(line);
	string s;
	while (getline(iss, s, ' ')) 
		dst.push_back(stod(s));
}

Rectifier::Rectifier(const std::string & fileName, double alpha)
{
	using namespace std;
	ifstream fs(fileName);
	string line;
	int idx = -1;
	int width, height;
	std::vector<double> distortion[2], intrinsics[2], projection[2], rectification[2];
	while (getline(fs, line)) {
		if (line == "[image]") {
			idx++;
			if (idx > 1) throw std::runtime_error("Unexpected [image]");
		}
		else if (line == "[narrow_stereo]" || line == "[narrow_stereo/left]") {
			if (idx != 0) throw std::runtime_error("Unexpected [narrow_stereo/...]");
			size[idx] = cv::Size(width, height);
		}
		else if (line == "[narrow_stereo/right]") {
			if (idx != 1) throw std::runtime_error("Unexpected [narrow_stereo/...]");
			size[idx] = cv::Size(width, height);
		}
		else if (line == "width") {
			if (getline(fs, line)) 
				width = stoi(line);
		}
		else if (line == "height") {
			if (getline(fs, line)) 
				height = stoi(line);
		}
		else if (line == "distortion") {
			if (getline(fs, line)) 
				parseLine(distortion[idx], line);
		}
		else if (line == "camera matrix") {
			for (int x = 3; x && getline(fs, line); x--) 
				parseLine(intrinsics[idx], line);
		}
		else if (line == "projection") {
			for (int x = 3; x && getline(fs, line); x--) 
				parseLine(projection[idx], line);
		}
		else if (line == "rectification") {
			for (int x = 3; x && getline(fs, line); x--) 
				parseLine(rectification[idx], line);
		}
	}
	fs.close();
	switch (idx) {
	case 0: {	
		cv::Mat intr(3, 3, CV_64F, intrinsics[0].data());		
		cv::Mat ncm = cv::getOptimalNewCameraMatrix(intr, distortion[0], size[0], alpha);
		cv::Mat r(3, 3, CV_64F, rectification[0].data());
		cv::initUndistortRectifyMap(intr, distortion[0], r, ncm, size[0], CV_32F, mapX[0], mapY[0]);
		break;
	}
	case 1: {
		cv::Mat intr0(3, 3, CV_64F, intrinsics[0].data());
		cv::Mat intr1(3, 3, CV_64F, intrinsics[1].data());
		cv::Mat r0(3, 3, CV_64F, rectification[0].data());
		cv::Mat r1(3, 3, CV_64F, rectification[1].data());
		cv::Mat p0(3, 4, CV_64F, projection[0].data());
		cv::Mat p1(3, 4, CV_64F, projection[1].data());

		cv::initUndistortRectifyMap(intr0, distortion[0], r0, p0, size[0], CV_32F, mapX[0], mapY[0]);
		cv::initUndistortRectifyMap(intr1, distortion[1], r1, p1, size[1], CV_32F, mapX[1], mapY[1]);
		break;
	}
	default:
		throw std::runtime_error("File format error");
	}
}

void Rectifier::rectify(const cv::Mat & src, cv::Mat & dst, int n)
{
	assert(n == 0 || n == 1);
	cv::remap(src, dst, mapX[n], mapY[n], cv::INTER_LINEAR);
}
