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
#include <deque>      
#include <vector> 
#include <fstream>

//Shark ML
/*#include <shark/Data/Dataset.h>
#include <shark/Algorithms/Trainers/RFTrainer.h> //the random forest trainer
#include <shark/ObjectiveFunctions/Loss/ZeroOneLoss.h> //zero one loss for evaluation
#include <shark/Data/Csv.h>*/

//#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;
using namespace shark;
using namespace glm;

bool run = true; //whether program continues to run
bool draw = true; //whether to output the opencv processed images
bool twoHands = false; //whether accomodates for 2 hands
int checkstep = 2;
bool freemove = false; //whether can move freely around the environment
int ksize = 45;
int sigma = 100;
int checkSize = 60;
int fps = 30;
int maxDepth = 1000;
int movement = 0;
unsigned long stepTime = GetTickCount(); //A Windows only function, gives time in milliseconds
int counter = 0;
int globalCounter = 0;
int numSavedPoses = 15;
int frameCheck = numSavedPoses;
string outputImageName = "OpenGL image";


fstream output;


pixel prevHand1;
deque<int> poses;
bool menuOpen = true;
int menuCounter = 0;
int menuClosedCounter = 0;

void main()
{
	namedWindow(outputImageName, WINDOW_AUTOSIZE);
	//int tbarval = 10;
	//createTrackbar("trackbar1", outputImageName, &tbarval, 100);

	output.open("data.csv", ios::app);//fstream::out);

	CDepthBasics cdepth;
	ML ml;
	App a;

	a.setCamera(0, 5, -25);
	a.setupView();
	a.setupPhysics();

	cdepth.SetUp(twoHands, ksize, sigma, checkSize, draw, checkstep, maxDepth); //MAKE SO CHECKS KINECT
	
	Cube cube1;
	a.addToWorld(cube1.setUpPhysics(btVector3(0,10,0)));

	Cube stillCube;
	a.addToWorld(stillCube.setUpPhysics(btVector3(0,1,0)));
	stillCube.setColour(1, 1, 0);
	stillCube.doBuffers();

	Menu2D menuBackground;
	Trackbar tbar1;
	Trackbar tbar2;
	Cursor cursor;
	Cursor fingertipCursor;
	fingertipCursor.setColour(1, 0, 0);
	fingertipCursor.doBuffers();

	Slider slider1;
	slider1.setX(-a.getWidth()*0.05 + 40);
	slider1.setY(a.getHeight()*0.05);

	Plane plane1;
	a.addToWorld(plane1.setUpPhysics(btVector3(0,1,0), btVector3(0,-1,0)));
	plane1.setColour(0.5, 0, 0.5);
	plane1.doBuffers();

	Plane rightWall;
	a.addToWorld(rightWall.setUpPhysics(btVector3(1, 0, 0), btVector3(-10, 0, 0)));
	rightWall.setColour(0.5, 0.5, 0);
	rightWall.doBuffers();
	
	Plane leftWall;
	a.addToWorld(leftWall.setUpPhysics(btVector3(-1, 0, 0), btVector3(10, 0, 0)));
	leftWall.setColour(0.5, 0.5, 0);
	leftWall.doBuffers();
	
	Plane backWall;
	a.addToWorld(backWall.setUpPhysics(btVector3(0, 0, -1), btVector3(0, 0, 10)));
	backWall.setColour(0.35, 0.6, 0.25);
	backWall.doBuffers();

	Plane closeWall;
	a.addToWorld(closeWall.setUpPhysics(btVector3(0, 0, 1), btVector3(0, 0, -15)));
	

	for (int i = 0; i < numSavedPoses; i++)
	{
		poses.push_back(0);
	}
	
	while (run && a.continueProcessing())
	{
		a.step(GetTickCount() - stepTime);
		stepTime = GetTickCount();

		cdepth.Update(); //Fills in depthArr
		cdepth.FindHands(false, checkstep, twoHands); //Find the position of the hand(s)

		pixel hand1 = cdepth.getHand1();
		if (globalCounter == 0) {
			prevHand1 = hand1;
		}

		pair<vector<Point>, Point> hand1Features = cdepth.ProcessHand(hand1, draw); //Get the information used for pose identification
		vector<Point> hand1hull = hand1Features.first;
		Point fingertip1 = hand1Features.second;
		
		int pose = ml.Predict(hand1hull, hand1); //Predict the pose

		/*if (draw)
		{
			cdepth.DrawClassification(pose);
			cdepth.Draw(2);
		}*/

		a.preprocessing(freemove);

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

		a.pushMat();
			if (pose == 2) { //Grabbing the cube
				a.applyForce(cube1, GetTickCount() - stepTime, (prevHand1.xpos - hand1.xpos) * 1000, (prevHand1.ypos - hand1.ypos) * 1000, (hand1.depth - prevHand1.depth) * 1000);
			}
			else if (poses.back() == 2 &&  abs(hand1.depth - prevHand1.depth) < 10 && abs(hand1.xpos - prevHand1.xpos) + abs(hand1.ypos - prevHand1.ypos)  < 10) { // because can fly off a little if opening hand due to movement of central point and change of depth
				cube1.getRigidBody()->setLinearVelocity(btVector3(0, 0, 0));
			}
			a.updateCube(cube1);
			a.render(cube1);
		a.popMat();

		a.pushMat();
			a.updateCube(stillCube);
			a.render(stillCube);
		a.popMat();

		a.pushMat();
			a.scale(10, 10, 10);
			a.render(plane1);
		a.popMat();

		a.pushMat();
			a.translate(rightWall.getRigidBody()->getCenterOfMassPosition()[0], rightWall.getRigidBody()->getCenterOfMassPosition()[1], rightWall.getRigidBody()->getCenterOfMassPosition()[2]);
			a.rotate(3.14159/2, 0, 0, 1);
			a.scale(10, 10, 10);
			a.render(rightWall);
		a.popMat();

		a.pushMat(); //remove these, change actual vertex data at start
			a.translate(leftWall.getRigidBody()->getCenterOfMassPosition()[0], leftWall.getRigidBody()->getCenterOfMassPosition()[1], leftWall.getRigidBody()->getCenterOfMassPosition()[2]);
			a.rotate(3.14159 / 2, 0, 0, 1);
			a.scale(10, 10, 10);
			a.render(leftWall);
		a.popMat();

		a.pushMat();
			a.translate(backWall.getRigidBody()->getCenterOfMassPosition()[0], backWall.getRigidBody()->getCenterOfMassPosition()[1], backWall.getRigidBody()->getCenterOfMassPosition()[2]);
			a.rotate(3.14159 / 2, 1, 0, 0);
			a.scale(10, 10, 10);
			a.render(backWall);
		a.popMat();

		if (menuOpen == true) { //stay open
			mat4 p = a.getP();
			double width = a.getWidth();
			double height = a.getHeight();
			a.setP(ortho(-width*0.1, width*0.1, -height*0.1, height*0.1, 0.0, 50.0)); //must be doubles or floats, smaller values give bigger square
			glDisable(GL_DEPTH_TEST);

			//background
			a.pushMat();
				a.scale(width*0.01 - 0.5, height*0.01 - 0.5, 0);
				a.render(menuBackground);
			a.popMat();

			//trackbar 1
			a.pushMat();
				a.translate(-width*0.05, height*0.05, 0);
				a.scale(5, 0.5, 0);
				a.render(tbar1);
			a.popMat();

			float cursorx = -(float)fingertip1.x;
			float cursory = -(float)fingertip1.y;
			float cursorxhand = -(float)hand1.xpos;
			float cursoryhand = -(float)hand1.ypos;
			float maxy = cdepth.getHeight();
			float maxx = cdepth.getWidth();
			float globalFingerX = (cursorxhand + maxx / 2 + (cursorx - cursorxhand) / 5) / 1.5;
			float globalFingerY = (cursoryhand + maxy / 2 + (cursory - cursoryhand) / 5) / 2.0;
			//slider 1
			a.pushMat();
				//+40 and +5 (not there if y size halved) come from the width of tbar scaled
				a.translate(slider1.getXPos(), height*0.05, 0); //move to starting position
				if (pose == 3 && slider1.isWithin(globalFingerX, globalFingerY)) {
					a.translate(-(slider1.getXPos()),0 , 0); //reset to original position
					a.translate(globalFingerX, 0, 0);
					slider1.setX(globalFingerX);
				}
				a.scale(0.4, 0.5, 0);
				a.render(slider1);
			a.popMat();

			//trackbar 2
			a.pushMat();
				a.translate(-width*0.05, -height*0.01, 0);
				a.scale(5, 0.5, 0);
				a.render(tbar2);
			a.popMat();

			//main cursor
			a.pushMat();
				a.translate((cursorxhand+maxx/2)/1.5, (cursoryhand+maxy/2)/2.0, 0);
				a.scale(0.2, 0.2, 0);
				a.render(cursor);
			a.popMat();

			//finger cursor
			if (pose == 3) {
				a.pushMat();
					a.translate(globalFingerX, globalFingerY, 0); //scale so finger tip appears closer
					a.scale(0.1, 0.1, 0);
					a.render(fingertipCursor);
				a.popMat();
			}

			glEnable(GL_DEPTH_TEST);
			a.setP(p);

			menuOpen = true;
			menuCounter++;
		}
		else {
			menuClosedCounter++;
		}
		
		if(a.menuOpenGesture(poses) && menuCounter > frameCheck){ //go from open to closed
			menuOpen = false;
			menuCounter = 0;
		}

		if (a.menuOpenGesture(poses) && menuOpen == false && menuClosedCounter > frameCheck) { //open from closed
			menuOpen = true;
			menuClosedCounter = 0;
		}

		a.postprocessing();

		globalCounter++;
		prevHand1 = hand1;
		poses.pop_front();
		poses.push_back(pose);

		//display output
		a.outputImage(outputImageName, cdepth.Draw(2));


		int key = waitKey((int)(1000 / fps));
		/*if (key == 'z')
		{
			string data = ml.getMLTrainData(hand1hull, hand1);
			output << "1," << data;
			cout << "pose 1" << endl;
		}
		else if (key == 'x')
		{
			string data = ml.getMLTrainData(hand1hull, hand1);
			output << "2," << data;
			cout << "pose 2" << endl;
		}
		else if (key == 'c')
		{
			string data = ml.getMLTrainData(hand1hull, hand1);
			output << "3," << data;
			cout << "pose 3" << endl;
		}*/
		if (key == 'p')
		{
			a.setPosition(stillCube, 0, 1, 0);
		}
		else if (key == 'q')
		{
			cdepth.FindHands(true, checkstep, twoHands);
		}
		else if (key != -1)
		{
			run = false;
		}

		
	}
	
}