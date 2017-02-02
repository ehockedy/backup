#pragma once

#include <opencv2/opencv.hpp>

struct pixel
{
	int xpos;
	int ypos;
	int depth;
};

float getDist(cv::Point p, pixel p2);
