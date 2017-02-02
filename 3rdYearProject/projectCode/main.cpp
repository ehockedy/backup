#pragma once

#include "stdafx.h"
#include <strsafe.h>
#include "resource.h"
#include "ml.h"
#include "header.h"
#include "DepthBasics2.h"

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

bool run = true;
bool draw = true;
bool twoHands = false;
int ksize = 45;
int sigma = 100;
int checkSize = 60;
int fps = 30;

void main()
{
	CDepthBasics cdepth;
	ML ml;
	
	cdepth.SetUp(twoHands, ksize, sigma, checkSize);

	while (run)
	{
		cdepth.Update(); //Fills in depthArr
		cdepth.FindHands(false); //Find the position of the hand(s)

		pixel hand1 = cdepth.getHand1();

		pair<vector<Point>, Point> hand1Features = cdepth.ProcessHand(hand1, draw); //Get the information used for pose identification
		vector<Point> hand1hull = hand1Features.first;
		Point fingertip1 = hand1Features.second;
		
		int pose = ml.Predict(hand1hull, hand1); //Predict the pose

		if (draw)
		{
			cdepth.DrawClassification(pose);
			cdepth.Draw();
		}

		int key = waitKey((int)(1000 / fps));
		if (key != -1)
		{
			run = false;
		}
	}
	
}