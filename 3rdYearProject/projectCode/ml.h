#pragma once

#include "resource.h"
//#include "ImageRenderer.h"
#include <fstream>
#include "header.h"

#include <opencv2/opencv.hpp>
#include <shark/Data/Dataset.h>
#include <shark/Algorithms/Trainers/RFTrainer.h> //the random forest trainer
#include <shark/ObjectiveFunctions/Loss/ZeroOneLoss.h> //zero one loss for evaluation


class ML
{

public:
	ML();
	//~ML();

	void Train();
	int Predict(std::vector<cv::Point> convexHull, pixel pix);

private:
	shark::RFClassifier model;
	int classify(shark::Data<shark::RealVector> prediction, int sampleNum, double confidenceThreshold); //Chooses class from 1 to n, or 0 if unsure, sampleNum is for is there is more than 1 piece of data, it chooses the sampleNumth piece, confidenceThreshold is the value, below which the unknown label is returned.
	shark::Data<shark::RealVector> getMLdata(std::vector<cv::Point> hullPoints, pixel centralPoint);
};