#pragma once

#include "stdafx.h"
#include <strsafe.h>
#include "resource.h"
#include "ml.h"
#include "header.h"
#include "DepthBasics2.h"
#include "application.h"

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
bool draw = false;
bool twoHands = false;
int ksize = 45;
int sigma = 100;
int checkSize = 60;
int fps = 30;
int movement = 0;

void main()
{
	CDepthBasics cdepth;
	ML ml;
	App a;

	a.setup();
	cdepth.SetUp(twoHands, ksize, sigma, checkSize, draw);

	Cube cube1 = Cube(1, 0, 0);

	while (run && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0)
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

		a.preprocessing();

		if (pose == 1) {
			cube1.setColour(1, 0, 0);
		}
		else if (pose == 2)		{
			cube1.setColour(0, 1, 0);
		}
		else if (pose == 3) {
			cube1.setColour(0, 0, 1);
		}
		else {
			cube1.setColour(0, 0, 0);
		}
		cube1.doBuffers();

		pushMat(Model);
			//Model = rotpos2;//MAY HAVE TO SCALE
			Model = translate(Model, vec3((cdepth.getWidth()/2 - hand1.xpos)*0.1, (cdepth.getHeight()/2 - hand1.ypos)*0.1, hand1.depth/10-25 ));
			MVP = Projection * View * Model;
			glUniformMatrix4fv(a.getMatrixID(), 1, GL_FALSE, &MVP[0][0]);
			renderAttrib(0, *cube1.getVertexBuffer());
			renderAttrib(1, *cube1.getColourBuffer());
			glDrawArrays(GL_TRIANGLES, 0, 12 * 3);
		Model = popMat();

		a.postprocessing();

		int key = waitKey((int)(1000 / fps));
		if (key != -1)
		{
			run = false;
		}
	}
	
}