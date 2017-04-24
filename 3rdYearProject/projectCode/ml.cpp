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
	ifstream ifs("trainedRF.model"); //Load the trained model
	boost::archive::polymorphic_text_iarchive ia(ifs);
	model.read(ia);
	ifs.close();
}

int ML::classify(Data<RealVector> prediction, int sampleNum, double confidenceThreshold) //Chooses class from 1 to n, or 0 if unsure, sampleNum is for if there is more than 1 piece of data, it chooses the sampleNumth piece, confidenceThreshold is the value, below which the unknown label is returned.
{
	double classificationVal = 0;
	int classification = 0;
	//prediction is the predictions of all of the n pieces of data. In the real time pose estimation, there is onlt 1 prediction (for each hand)
	for (int i = 0; i < prediction.element(sampleNum).size(); i++)
	{
		//prediction.element(sampleNum)[i] contains values between 0 and 1 for each class that indicated the percentage of trees that voted for the ith class
		//The total of the values is 1
		//This loop find the class that has the highest percentage and as such has been voted for the most by the forest
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

//Get data into a format that can be used for training or classifying 
//Use numPoints points as this is normally just above the maximum. Usually 25
Data<RealVector> ML::getMLdata(vector<Point> hullPoints, pixel centralPoint)
{
	float numPoints = 25.0;
	double mult = ((double)centralPoint.depth / 255.0)*0.5 + 0.5; //Scale because using distances. Doesn't work perfectly
	RealVector data = {};
	if (hullPoints.size() >= numPoints) //If there are 25 or more points, use the first 25
	{
		for (int i = 0; i < numPoints; i++)
		{
			data.push_back(getDist(hullPoints[i], centralPoint) / mult);
			//data.push_back(getAngle(hullPoints[i], centralPoint));

		}
	}
	else
	{
		int j = 0; //The current position in the convexHull list
		float count = 0; //The value that i must be greater than in order for a ? to be included where ? is an interpolated point
		float gap = numPoints / (numPoints - hullPoints.size()); //The gap between ? on average

		for (int i = 0; i < numPoints; i++)
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
	//lebl is in the first column, a comma is the character separating the values
	importCSV(data, "data.csv", FIRST_COLUMN, ',');
	data.shuffle();
	cout << "Data imported" << endl;

	RFTrainer trainer;
	RFClassifier model;

	//trainer.setNodeSize(20);
	trainer.setNTrees(100);
	
	trainer.train(model, data);
	cout << "Training done" << endl;
	ofstream ofs("trainedRF.model");
	boost::archive::polymorphic_text_oarchive oa(ofs);

	cout << "Writing" << endl;
	model.write(oa);
	ofs.close();
}

int ML::Predict(vector<Point> convexHull, pixel pix)
{
	Data<RealVector> poseData = getMLdata(convexHull, pix);
	Data<RealVector> prediction = model(poseData);
	int pose = classify(prediction, 0, 0); //0.5
	//putText(imgD, to_string(pose), Point(30, 50), FONT_HERSHEY_SIMPLEX, 2, Scalar(0, 0, 255), 3);
	return pose;
}

//Same as other function, but writes to the data file
string ML::getMLTrainData(vector<Point> hullPoints, pixel centralPoint)
{
	double mult =  ((double)centralPoint.depth / 255.0)*0.5 + 0.5;
	ostringstream ss;
	string data = "";
	if (hullPoints.size() >= 25)
	{
		for (int i = 0; i < 25; i++)
		{
			ss << getDist(hullPoints[i], centralPoint) / mult;// << ',' << getAngle(hullPoints[i], centralPoint);
			string s(ss.str());
			data += s;
			data += ',';
			ss.str(string());
			ss.clear();
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
				ss << avg / mult;// << ',' << getAngle(hullPoints[j], centralPoint);;
				string s(ss.str());
				data += s;
				data += ',';
				ss.str(string());
				ss.clear();
				count += gap;
			}
			else
			{
				ss << getDist(hullPoints[j], centralPoint) / mult;// << ',' << getAngle(hullPoints[j], centralPoint);;
				string s(ss.str());
				data += s;
				data += ',';
				ss.str(string());
				ss.clear();
				j++;
			}
		}
	}

	data.pop_back(); //Get rid of the final comma
	data += '\n';
	return data;
}