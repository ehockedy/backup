#pragma once

#include "stdafx.h"
#include <strsafe.h>
#include "resource.h"
#include "ml.h"
#include "header.h"
#include <fstream>

//Shark ML
#include <shark/Data/Dataset.h>
#include <shark/Algorithms/Trainers/RFTrainer.h> //the random forest trainer
#include <shark/ObjectiveFunctions/Loss/ZeroOneLoss.h> //zero one loss for evaluation
#include <shark/Data/Csv.h>

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;
using namespace shark;


ML::ML()
{
	ifstream ifs("trainedRF.model");
	boost::archive::polymorphic_text_iarchive ia(ifs);
	model.read(ia);
	ifs.close();
}

int ML::classify(Data<RealVector> prediction, int sampleNum, double confidenceThreshold) //Chooses class from 1 to n, or 0 if unsure, sampleNum is for is there is more than 1 piece of data, it chooses the sampleNumth piece, confidenceThreshold is the value, below which the unknown label is returned.
{
	double classificationVal = 0;
	int classification = 0;
	for (int i = 0; i < prediction.element(sampleNum).size(); i++)
	{
		//cout << prediction.element(sampleNum)[i] << endl;
		if (prediction.element(sampleNum)[i] > classificationVal)
		{
			classificationVal = prediction.element(sampleNum)[i];
			classification = i;
		}
	}
	if (classificationVal < confidenceThreshold)
	{
		classification = -1;
	}
	return classification + 1;
}


Data<RealVector> ML::getMLdata(vector<Point> hullPoints, pixel centralPoint)
{
	double mult = ((double)centralPoint.depth / 255.0)*0.5 + 0.5;
	RealVector data = {};
	if (hullPoints.size() >= 25)
	{
		for (int i = 0; i < 25; i++)
		{
			data.push_back(getDist(hullPoints[i], centralPoint) / mult);
			//data.push_back(getAngle(hullPoints[i], centralPoint));

		}
	}
	else
	{
		int j = 0; //The current position in the convexHull list
		float count = 0; //The value that i must be greater than in order for a ? to be included
		float gap = 25.0 / (25.0 - hullPoints.size()); //The gap between ? on average

		for (int i = 0; i < 25; i++)
		{
			if (i >= count) //Interpolate such that we get a total of 25 features. Interpolation might work since the area around the hand is fairly continuous
			{
				float avg = (getDist(hullPoints[j % hullPoints.size()], centralPoint) + getDist(hullPoints[(j + 1) % hullPoints.size()], centralPoint)) / 2;
				data.push_back(avg / mult);
				//data.push_back(getAngle(hullPoints[j], centralPoint));
				count += gap;
			}
			else
			{
				//PRINT(data.size());
				data.push_back(getDist(hullPoints[j], centralPoint) / mult);
				//data.push_back(getAngle(hullPoints[j], centralPoint));
				j++;
			}
		}
	}
	vector<RealVector> inputData = { data };
	Data<RealVector> dataOut = createDataFromRange(inputData);
	return dataOut;
}

void ML::Train()
{
	ClassificationDataset data;
	importCSV(data, "data.csv", FIRST_COLUMN, ',');
	data.shuffle();

	RFTrainer trainer;
	RFClassifier model;
	trainer.train(model, data);

	ofstream ofs("trainedRF.model");
	boost::archive::polymorphic_text_oarchive oa(ofs);
	model.write(oa);
	ofs.close();
}

int ML::Predict(vector<Point> convexHull, pixel pix)
{
	Data<RealVector> poseData = getMLdata(convexHull, pix);
	Data<RealVector> prediction = model(poseData);
	int pose = classify(prediction, 0, 0.5);
	//putText(imgD, to_string(pose), Point(30, 50), FONT_HERSHEY_SIMPLEX, 2, Scalar(0, 0, 255), 3);
	return pose;
}