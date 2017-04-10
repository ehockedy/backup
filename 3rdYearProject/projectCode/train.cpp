#pragma once

#include "stdafx.h"
#include <strsafe.h>
#include "resource.h"
#include "ml.h"
#include "header.h"
#include "DepthBasics2.h"
//#include "application.h"
#include "environment.h"
#include <fstream>
#include <queue>      
#include <vector> 
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

bool run2 = true; //whether program continues to run
bool draw2 = true; //whether to output the opencv processed images
bool twoHands2 = false; //whether accomodates for 2 hands
int checkstep2 = 1;
int ksize2 = 45;
int sigma2 = 100;
int checkSize2 = 60;
int fps2 = 30;
int maxDepth2 = 1000;
int movement2 = 0;
int counter2 = 0;
int globalCounter2 = 0;

fstream output2;


pixel prevHand12;

void main2()
{
	output2.open("data.csv", ios::app);//fstream::out);

	CDepthBasics cdepth;
	ML ml;
	cdepth.SetUp(twoHands2, ksize2, sigma2, checkSize2, draw2, checkstep2, maxDepth2);

	while (run2)
	{
		cdepth.Update(); //Fills in depthArr
		cdepth.FindHands(false, checkstep2, twoHands2); //Find the position of the hand(s)

		pixel hand1 = cdepth.getHand1();
		if (globalCounter2 == 0) {
			prevHand12 = hand1;
		}

		pair<vector<Point>, Point> hand1Features = cdepth.ProcessHand(hand1, draw2, 1); //Get the information used for pose identification
		vector<Point> hand1hull = hand1Features.first;
		Point fingertip1 = hand1Features.second;

		int pose = ml.Predict(hand1hull, hand1); //Predict the pose

		if (draw2)
		{
			cdepth.DrawLeftClassification(pose);
			cdepth.DrawTrain(1);
		}


		int key = waitKey((int)(1000 / fps2));
		if (key == 'z')
		{
			string data = ml.getMLTrainData(hand1hull, hand1);
			output2 << "1," << data;
			cout << "pose 1" << endl;
		}
		else if (key == 'x')
		{
			string data = ml.getMLTrainData(hand1hull, hand1);
			output2 << "2," << data;
			cout << "pose 2" << endl;
		}
		else if (key == 'c')
		{
			string data = ml.getMLTrainData(hand1hull, hand1);
			output2 << "3," << data;
			cout << "pose 3" << endl;
		}
		else if (key == 't') {
			ml.Train();
		}
		else if (key != -1)
		{
			run2 = false;
		}

	}


}
