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

//Shark ML
#include <shark/Data/Dataset.h>
#include <shark/Algorithms/Trainers/RFTrainer.h> //the random forest trainer
#include <shark/ObjectiveFunctions/Loss/ZeroOneLoss.h> //zero one loss for evaluation
#include <shark/Data/Csv.h>

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;
using namespace shark;
using namespace glm;

bool run = true; //whether program continues to run
bool draw = true; //whether to output the opencv processed images
bool twoHands = false; //whether accomodates for 2 hands
int checkstep = 1;
bool freemove = false; //whether can move freely around the environment
int ksize = 45;
int sigma = 100;
int checkSize = 60;
int fps = 30;
int movement = 0;
unsigned long stepTime = GetTickCount(); //A Windows only function, gives time in milliseconds
int counter = 0;
int globalCounter = 0;

pixel prevHand1;
queue<int> poses;

void main()
{
	cout << 1;
	CDepthBasics cdepth;
	ML ml;
	App a;
	cout << 2;
	a.setCamera(0, 5, -25);
	a.setupView();
	a.setupPhysics();
	cout << 3;
	cdepth.SetUp(twoHands, ksize, sigma, checkSize, draw, checkstep);
	cout << 4;
	Cube cube1;
	a.addToWorld(cube1.setUpPhysics());
	cout << 5;
	Plane plane1;
	a.addToWorld(plane1.setUpPhysics(btVector3(0,1,0), btVector3(0,-1,0)));
	plane1.setColour(0.5, 0, 0.5);
	plane1.doBuffers();
	cout << 6;
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

	//cout << plane1.getRigidBody()->isKinematicObject() << " ";
	//cout << plane1.getRigidBody()->isStaticObject() << endl;
	/*btTransform t;
	plane1.getRigidBody()->getMotionState()->getWorldTransform(t);
	t.setRotation(btQuaternion(btVector3(1, 0, 1), 3.14 / 4));
	t.setOrigin(btVector3(0, -10, 0));
	plane1.getRigidBody()->getMotionState()->setWorldTransform(t);*/
	//maybe cant move because static, make kinematic?
	

	for (int i = 0; i < 10; i++)
	{
		poses.push(0);
	}
	
	while (run && a.continueProcessing())
	{
		cout << counter << endl;
		//if (counter < 20) {
			a.step(GetTickCount() - stepTime);
			stepTime = GetTickCount();
			//counter++;
		//}
		counter++;
		counter = counter % 70;

		cdepth.Update(); //Fills in depthArr
		cdepth.FindHands(false, checkstep); //Find the position of the hand(s)

		pixel hand1 = cdepth.getHand1();
		if (globalCounter == 0) {
			prevHand1 = hand1;
		}

		pair<vector<Point>, Point> hand1Features = cdepth.ProcessHand(hand1, draw); //Get the information used for pose identification
		vector<Point> hand1hull = hand1Features.first;
		Point fingertip1 = hand1Features.second;
		
		int pose = ml.Predict(hand1hull, hand1); //Predict the pose

		if (draw)
		{
			cdepth.DrawClassification(pose);
			cdepth.Draw(3);
		}

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
			else if (poses.back() == 2 && abs(hand1.xpos - prevHand1.xpos) + abs(hand1.ypos - prevHand1.ypos) + abs(hand1.depth - prevHand1.depth) < 15) { //<15 because can fly off a little if opening hand due to movement of central point and change of depth
				cube1.getRigidBody()->setLinearVelocity(btVector3(0, 0, 0));
			}
			a.updateCube(cube1);
			a.render(cube1);
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

		a.pushMat();
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

		a.postprocessing();

		globalCounter++;
		prevHand1 = hand1;
		poses.pop();
		poses.push(pose);

		int key = waitKey((int)(1000 / fps));
		if (key != -1)
		{
			run = false;
		}
	}
	
}