#pragma once

#include <string>
#include <opencv2/core.hpp>

class Rectifier
{
	cv::Mat mapX[2], mapY[2];
public:
	cv::Size size[2];
	Rectifier(const std::string &fileName, double alpha = 0.0);
	void rectify(const cv::Mat & src, cv::Mat & dst, int n = 0);
};
