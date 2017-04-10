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
#include <cmath>

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

bool firstFrame = true;
bool run = true; //whether program continues to run
bool draw = true; //whether to output the opencv processed images
bool twoHands = false; //whether accomodates for 2 hands
int checkstep = 2;
bool freemove = false; //whether can move freely around the environment
int ksize = 45;
int sigma = 100;
int checkSize = 50;
int fps = 120;
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
pixel prevPrevHand1;
pixel prevHand2;
deque<int> rightPoses;
deque<int> leftPoses;
bool menuOpen = false;
int menuCounter = 0;
int menuClosedCounter = 0;
bool grabbing = false;
vector<Cube*> cubes;
vector<int> cubeState;
vector<int> leftCubeState;
int mostRecentCube = 0;
float offsetx = 0;
float offsety = 0;
float offsetz = 0;
float offsetx2 = 0;
float offsety2 = 0;
float offsetz2 = 0;
float xRoomSize = 15;
float zRoomSizeBack = 10;
float zRoomSizeFront = 15;

float startMass = 1;
float startxdim = 1.0;
float startydim = 1.0;
float startzdim = 1.0;

float axisSize = 0.02;

float currentX = 0;
float currentY = 0;
float currentZ = 0;

float internalX = 0;
float internalY = 0;
float internalZ = 0;

int currentCube = 7;

float currentXLeft = 0;
float currentYLeft = 0;
float currentZLeft = 0;

float internalXLeft = 0;
float internalYLeft = 0;
float internalZLeft = 0;

int currentCubeLeft = 4;

int prevCube = -1;
int prevCubeLeft = -1;

void main()
{
	namedWindow(outputImageName, WINDOW_AUTOSIZE);

	output.open("data.csv", ios::app);//fstream::out);

	CDepthBasics cdepth;
	ML ml;
	App a;

	a.setCamera(0, 5, -25);
	a.setupView();
	a.setupPhysics();

	cdepth.SetUp(twoHands, ksize, sigma, checkSize, draw, checkstep, maxDepth); //MAKE SO CHECKS KINECT

	a.makeCube(0, 10, -5, startMass, startxdim, startydim, startzdim, &cubes, &cubeState, &leftCubeState);
	a.makeCube(0, 5, -5, startMass, startxdim, startydim, startzdim, &cubes, &cubeState, &leftCubeState);
	a.makeCube(0, 1, -5, startMass, startxdim, startydim, startzdim, &cubes, &cubeState, &leftCubeState);
	a.makeCube(2, 10, -2, startMass, startxdim, startydim, startzdim, &cubes, &cubeState, &leftCubeState);
	a.makeCube(2, 5, -2, startMass, startxdim, startydim, startzdim, &cubes, &cubeState, &leftCubeState);
	a.makeCube(2, 1, -2, startMass, startxdim, startydim, startzdim, &cubes, &cubeState, &leftCubeState);
	a.makeCube(-2, 10, -2, startMass, startxdim, startydim, startzdim, &cubes, &cubeState, &leftCubeState);
	a.makeCube(-2, 5, -2, startMass, startxdim, startydim, startzdim, &cubes, &cubeState, &leftCubeState);
	a.makeCube(-2, 1, -2, startMass, startxdim, startydim, startzdim, &cubes, &cubeState, &leftCubeState);
	a.makeCube(-4, 10, 2, startMass, startxdim, startydim, startzdim, &cubes, &cubeState, &leftCubeState);
	a.makeCube(-4, 5, 2, startMass, startxdim, startydim, startzdim, &cubes, &cubeState, &leftCubeState);
	a.makeCube(-4, 1, 2, startMass, startxdim, startydim, startzdim, &cubes, &cubeState, &leftCubeState);
	a.makeCube(4, 10, 2, startMass, startxdim, startydim, startzdim, &cubes, &cubeState, &leftCubeState);
	a.makeCube(4, 5, 2, startMass, startxdim, startydim, startzdim, &cubes, &cubeState, &leftCubeState);
	a.makeCube(4, 1, 2, startMass, startxdim, startydim, startzdim, &cubes, &cubeState, &leftCubeState);
	a.makeCube(0, 1, -10, startMass, startxdim, startydim, startzdim, &cubes, &cubeState, &leftCubeState);
	a.makeCube(3, 1, -10, startMass, startxdim, startydim, startzdim, &cubes, &cubeState, &leftCubeState);
	a.makeCube(-3, 1, -10, startMass, startxdim, startydim, startzdim, &cubes, &cubeState, &leftCubeState);

	Cube xaxis;

	Menu2D menuBackground;
	Trackbar tbar1;
	a.addToMenuWorld(tbar1.setUpPhysics(btVector3(0, 30, 2)));//70
	Trackbar tbar2;
	a.addToMenuWorld(tbar2.setUpPhysics(btVector3(0, -10, 2)));//70

	Cursor cursor;
	a.addToMenuWorld(cursor.setUpPhysics(btVector3(0, 5, 2)));
	Cursor fingertipCursor;
	fingertipCursor.setColour(1, 0, 0);
	fingertipCursor.doBuffers();

	Cube cursor3dRight;
	cursor3dRight.setColour(0.3, 0.8, 1);
	cursor3dRight.doBuffers();

	Cube cursor3dLeft;
	cursor3dLeft.setColour(1, 0.8, 0.3);
	cursor3dLeft.doBuffers();
	
	Cube prevCursor3d;

	Slider slider1;
	a.addToMenuWorld(slider1.setUpPhysics(btVector3(20, 10, 0)));//25
	Slider slider2;
	a.addToMenuWorld(slider2.setUpPhysics(btVector3(0, -30, 0)));//25

	Plane plane1;
	a.addToWorld(plane1.setUpPhysics(btVector3(0,1,0), btVector3(0,-1,0)));///y=-1
	plane1.setColour(0.5, 0, 0.5);
	plane1.doBuffers();

	Plane rightWall;
	a.addToWorld(rightWall.setUpPhysics(btVector3(1, 0, 0), btVector3(-xRoomSize, 0, 0)));
	rightWall.setColour(0.5, 0.5, 0);
	rightWall.doBuffers();
	
	Plane leftWall;
	a.addToWorld(leftWall.setUpPhysics(btVector3(-1, 0, 0), btVector3(xRoomSize, 0, 0)));
	leftWall.setColour(0.5, 0.5, 0);
	leftWall.doBuffers();
	
	Plane backWall;
	a.addToWorld(backWall.setUpPhysics(btVector3(0, 0, -1), btVector3(0, 0, zRoomSizeBack)));
	backWall.setColour(0.35, 0.6, 0.25);
	backWall.doBuffers();

	Plane closeWall;
	a.addToWorld(closeWall.setUpPhysics(btVector3(0, 0, 1), btVector3(0, 0, -zRoomSizeFront)));
	
	for (int i = 0; i < numSavedPoses; i++)
	{
		rightPoses.push_back(0);
		leftPoses.push_back(0);
	}

	pixel hand1 = cdepth.getHand1();
	pixel hand2 = cdepth.getHand2();
	pair<vector<Point>, Point> leftHand;
	pair<vector<Point>, Point> rightHand;
	//bool leftIs1 = false;

	if (hand1.xpos < hand2.xpos && twoHands) { //if hand1 is on left of hand2 then swap, since always want hand1 to be right
		pixel hand3 = hand2; //swap them over
		hand2 = hand1;
		hand1 = hand3;
		cdepth.UpdatePreviousHands(hand1, hand2);
	}

	if(twoHands) {
		leftHand = cdepth.ProcessHand(hand2, draw, 2); //Get the information used for pose identification
		rightHand = cdepth.ProcessHand(hand1, draw, 1); 
	}
	else {
		rightHand = cdepth.ProcessHand(hand1, draw, 1); 
	}

	currentX = hand1.xpos;
	currentY = hand1.ypos;
	currentZ = hand1.depth;
	currentXLeft = hand2.xpos;
	currentYLeft = hand2.ypos;
	currentZLeft = hand2.depth;
	if (twoHands == false) {
		currentCubeLeft = -1;
	}

	vector<Point> leftHandHull;
	Point fingertip1;

	vector<Point> rightHandHull;
	Point fingertip2;

	int leftPose;
	int rightPose;

	while (run && a.continueProcessing())
	{
		counter = (counter + 1)%20 +1;
		
		stepTime = GetTickCount();

		if (firstFrame != true) {
			cdepth.Update(); //Fills in depthArr
			if (counter == 0) {
				cdepth.FindHands(true, checkstep, twoHands); //Find the position of the hand(s)
			}
			else {
				cdepth.FindHands(false, checkstep, twoHands); //Find the position of the hand(s)
			}
		}

		if (twoHands) {
			if (firstFrame != true) {
				hand1 = cdepth.getHand1();
				hand2 = cdepth.getHand2();

				rightHand = cdepth.ProcessHand(hand1, draw, 1); //Get the information used for pose identification
				leftHand = cdepth.ProcessHand(hand2, draw, 2); //Get the information used for pose identification
			}
			if (globalCounter == 0) {
				prevHand1 = hand1;
			}

			leftHandHull = leftHand.first;
			fingertip1 = leftHand.second;

			rightHandHull = rightHand.first;
			fingertip2 = rightHand.second;

			rightPose = ml.Predict(rightHandHull, hand1); //Predict the pose
			leftPose = ml.Predict(leftHandHull, hand2); //Predict the pose


		}
		else {
			hand1 = cdepth.getHand1();
			if (globalCounter == 0) {
				prevHand1 = hand1;
			}
			rightHand = cdepth.ProcessHand(hand1, draw, 1); //Get the information used for pose identification
			rightHandHull = rightHand.first;
			fingertip1 = rightHand.second;
			rightPose = ml.Predict(rightHandHull, hand1); //Predict the pose
		}

		if (draw)
		{
			cdepth.DrawRightClassification(rightPose);
			if (twoHands) {
				cdepth.DrawLeftClassification(leftPose);
			}
			//cdepth.Draw(2);
		}

		a.preprocessing(freemove);

		//Always render the room
		a.pushMat();
		a.scale(10, 10, 10);
		a.render(plane1);
		a.popMat();

		a.pushMat();
		a.translate(rightWall.getRigidBody()->getCenterOfMassPosition()[0], rightWall.getRigidBody()->getCenterOfMassPosition()[1], rightWall.getRigidBody()->getCenterOfMassPosition()[2]);
		a.rotate(-3.14159 / 2, 0, 0, 1); //- to make sure normals face inwards
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
		a.rotate(-3.14159 / 2, 1, 0, 0);
		a.scale(10, 10, 10);
		a.render(backWall);
		a.popMat();

		int sensitivityScale = 10.0; //higher value, more need to move hand
		/*if (menuOpen == false && rightPose == 3 && rightPoses[rightPoses.size() - 3] == 3 && rightPoses[rightPoses.size() - 5] == 3 && rightPoses[rightPoses.size() - 7] == 3) {
			offsetx = -((float)cdepth.getWidth() / 2.0 - (float)hand1.xpos) / sensitivityScale; //the distance from the original position, used when resetting position
			offsety = -((float)cdepth.getHeight() / 2.0 - (float)hand1.ypos - 50) / sensitivityScale;
			offsetz = ((cdepth.getMaxDepth() - cdepth.getMinDepth()) / 2.0 - 80 - (float)hand1.depth + offsetz) / sensitivityScale;
		}
		cursor3dRight.setXYZ(((float)cdepth.getWidth() / 2.0 - (float)hand1.xpos) / sensitivityScale + offsetx, ((float)cdepth.getHeight() / 2.0 - (float)hand1.ypos + offsety) / sensitivityScale + offsety, -((cdepth.getMaxDepth() - cdepth.getMinDepth()) / 2.0 - 80 - (float)hand1.depth + offsetz) / (sensitivityScale / 1.5) + offsetz);

		if (twoHands) {
			if (menuOpen == false && leftPose == 3 && leftPoses[leftPoses.size() - 3] == 3 && leftPoses[leftPoses.size() - 5] == 3 && leftPoses[leftPoses.size() - 7] == 3) {
				offsetx2 = -((float)cdepth.getWidth() / 2.0 - (float)hand2.xpos) / sensitivityScale;
				offsety2 = -((float)cdepth.getHeight() / 2.0 - (float)hand2.ypos - 50) / sensitivityScale;
				offsetz2 = ((cdepth.getMaxDepth() - cdepth.getMinDepth()) / 2.0 - 80 - (float)hand2.depth + offsetz2) / sensitivityScale;
			}
			cursor3dLeft.setXYZ(((float)cdepth.getWidth() / 2.0 - (float)hand2.xpos) / sensitivityScale + offsetx2, ((float)cdepth.getHeight() / 2.0 - (float)hand2.ypos + offsety2) / sensitivityScale + offsety2, -((cdepth.getMaxDepth() - cdepth.getMinDepth()) / 2.0 - 80 - (float)hand2.depth + offsetz2) / (sensitivityScale / 1.5) + offsetz2);
		}

		bool cubeChosen = false;
		bool leftCubeChosen = false;
		bool rightCubeChosen = false;
		for (int c = 0; c < cubes.size(); c++) {
			Cube* cube = cubes[c];
			if (menuOpen == false)
			{
				float range = cube->getScaleFactor() + 1;

				//letting go of cube
				//if (cubeState[c] == 1 && rightPose != 2 && rightPoses.back() != 2 && rightPoses[rightPoses.size() - 2] == 2 && abs(prevHand1.depth - prevPrevHand1.depth) < 15 && abs(prevHand1.xpos - prevPrevHand1.xpos) + abs(prevHand1.ypos - prevPrevHand1.ypos) < 10) { // because can fly off a little if opening hand due to movement of central point and change of depth
				if (cubeState[c] == 1 && rightPose != 2 && rightPoses.back() == 2 && abs(hand1.depth - prevHand1.depth) < 15 && abs(hand1.xpos - prevHand1.xpos) + abs(hand1.ypos - prevHand1.ypos) < 10) { //15, 10 are original because can fly off a little if opening hand due to movement of central point and change of depth
					cube->getRigidBody()->setLinearVelocity(btVector3(0, 0, 0));
					cube->getRigidBody()->setAngularVelocity(btVector3(0, 0, 0));
					cubeState[c] = 0; //not doing anything
					//cubeChosen = false;////
				}
				else if (rightPose != 2) { //throwing cube
					cubeState[c] = 0;
					//cubeChosen = false;////
				}
				else if (cubeState[c] == 1) { //In state of being held
					a.applyForce(*cube, GetTickCount() - stepTime, (prevHand1.xpos - hand1.xpos) * 1000, (prevHand1.ypos - hand1.ypos) * 1000, (hand1.depth - prevHand1.depth) * 1000);
				}
				else if ((rightPose == 2 && rightPoses.back() != 2 && prevCursor3d.within(*cube, range, range, range) && cubeChosen == false)) { //Grabbing the cube, use previous cursor position because cursor moves when grabbed
					cubeState[c] = 1; //1 = being grabbed
					cubeChosen = true;
					mostRecentCube = c;
				}

				if (twoHands) {
					if (leftCubeState[c] == 1 && leftPose != 2 && leftPoses.back() == 2 && abs(hand2.depth - prevHand2.depth) < 15 && abs(hand2.xpos - prevHand2.xpos) + abs(hand2.ypos - prevHand2.ypos) < 10) { // because can fly off a little if opening hand due to movement of central point and change of depth
						cube->getRigidBody()->setLinearVelocity(btVector3(0, 0, 0));
						cube->getRigidBody()->setAngularVelocity(btVector3(0, 0, 0));
						leftCubeState[c] = 0; //not doing anything
						//leftCubeChosen = false;////
					}
					else if (leftPose != 2) { //throwing cube
						leftCubeState[c] = 0;
						//leftCubeChosen = false;////
					}
					else if (leftCubeState[c] == 1) { //In state of being held
						a.applyForce(*cube, GetTickCount() - stepTime, (prevHand2.xpos - hand2.xpos) * 1000, (prevHand2.ypos - hand2.ypos) * 1000, (hand2.depth - prevHand2.depth) * 1000);
					}
					else if ((leftPose == 2 && leftPoses.back() != 2 && cursor3dLeft.within(*cube, range, range, range) && leftCubeChosen == false)) { //Grabbing the cube, use previous cursor position because cursor moves when grabbed
						leftCubeState[c] = 1; //1 = being grabbed
						leftCubeChosen = true;
						mostRecentCube = c;
					}

					if (leftCubeState[c] == 1 && cubeState[c] == 1) { //cube grabbed by both hands at once
						a.applyNegativeGravity(*cube); //Since anti gravity force is applied for each hand, this negates the effect of doublel anti-gravity
						float distance = abs((float)hand1.xpos - (float)hand2.xpos) / 200; //moving away in x direction grows cube
						btCollisionShape* shape = new btBoxShape(btVector3(distance, distance, distance));
						cube->getRigidBody()->setCollisionShape(shape);
						cube->setScaleFactor(distance);
					}

					cube->setXYZ(cube->getRigidBody()->getCenterOfMassPosition()[0], cube->getRigidBody()->getCenterOfMassPosition()[1], cube->getRigidBody()->getCenterOfMassPosition()[2]);


					//both hands interacing with the same cube
					if ((leftCubeChosen == false && cubeChosen == false && cursor3dLeft.within(*cube, range, range, range) && cursor3dRight.within(*cube, range, range, range))
						|| (leftCubeState[c] == 1 && cubeState[c] == 1)
						|| (leftCubeState[c] == 1 && cursor3dRight.within(*cube, range, range, range) && cubeChosen == false)
						|| (cubeState[c] == 1 && cursor3dLeft.within(*cube, range, range, range) && leftCubeChosen == false)) {
						cube->setColour(0.5, 0.5, 0.5);
						cube->doBuffers();
						leftCubeChosen = true;
						cubeChosen = true;
						mostRecentCube = c;
					}
					else if ((leftCubeChosen == false && cursor3dLeft.within(*cube, range, range, range)) || leftCubeState[c] == 1) {
						cube->setColour(0, 0, 0);
						cube->doBuffers();
						leftCubeChosen = true;
						mostRecentCube = c;
						a.pushMat();
						a.translate(cube->xpos, cube->ypos, cube->zpos);
						a.scale(axisSize, 100, axisSize);
						a.render(xaxis);
						a.popMat();

					}
					else if ((cubeChosen == false && cursor3dRight.within(*cube, range, range, range)) || cubeState[c] == 1) {
						cube->setColour(1, 1, 1);
						cube->doBuffers();
						cubeChosen = true;
						mostRecentCube = c;

						a.pushMat();
						a.translate(cube->xpos, cube->ypos, cube->zpos);
						a.scale(axisSize, 100, axisSize);
						a.render(xaxis);
						a.popMat();

					}
					else {
						cube->setColour(1, 1, 0);
						cube->doBuffers();
					}
				}

				if (twoHands == false) {
					cube->setXYZ(cube->getRigidBody()->getCenterOfMassPosition()[0], cube->getRigidBody()->getCenterOfMassPosition()[1], cube->getRigidBody()->getCenterOfMassPosition()[2]);

					if ((cubeChosen == false && cursor3dRight.within(*cube, range, range, range)) || cubeState[c] == 1) {
						//cube->setColour(1, 1, 1);
						//cube->doBuffers();
						cubeChosen = true;
						mostRecentCube = c;

						a.pushMat();
						a.translate(cube->xpos, cube->ypos, cube->zpos);
						a.scale(axisSize, 100, axisSize);
						a.render(xaxis);
						a.popMat();
					}
					else {
						cube->setColour(1, 1, 0);
						cube->doBuffers();
					}
				}
			}
			a.pushMat();
			a.updateCube(*cube);
			a.scale(cube->getScaleFactor(), cube->getScaleFactor(), cube->getScaleFactor()); //If double cube size, scale double as well, but after updated cube
			a.render(*cube);
			a.popMat();

			//cube->setXYZ(cube->getRigidBody()->getCenterOfMassPosition()[0], cube->getRigidBody()->getCenterOfMassPosition()[1], cube->getRigidBody()->getCenterOfMassPosition()[2]);

			if (cubeState[c] == 1) {
				xaxis.setColour(1, 0, 0);
				xaxis.doBuffers();
				a.pushMat();
				a.translate(cube->xpos, cube->ypos, cube->zpos);
				a.scale(axisSize, 100, axisSize);
				a.render(xaxis);
				a.popMat();
			}
		}*/

		/*if (cubeChosen == false && menuOpen == false) { //cant go in earlier if since must check cubeChosen after all have been iterated through
			a.pushMat();
			a.translate(cursor3dRight.xpos, cursor3dRight.ypos, cursor3dRight.zpos);
			a.scale(0.4, 0.4, 0.4);
			a.render(cursor3dRight);
			a.popMat();

			a.pushMat();
			a.translate(cursor3dRight.xpos, cursor3dRight.ypos, cursor3dRight.zpos);
			a.scale(axisSize, 100, axisSize);
			a.render(xaxis);
			a.popMat();

			a.pushMat();
			a.translate(cursor3dRight.xpos, cursor3dRight.ypos, cursor3dRight.zpos);
			a.scale(100, axisSize, axisSize);
			a.render(xaxis);
			a.popMat();

			a.pushMat();
			a.translate(cursor3dRight.xpos, cursor3dRight.ypos, cursor3dRight.zpos);
			a.scale(axisSize, axisSize, 100);
			a.render(xaxis);
			a.popMat();
		}
		if (leftCubeChosen == false && menuOpen == false && twoHands) {
			a.pushMat();
			a.translate(cursor3dLeft.xpos, cursor3dLeft.ypos, cursor3dLeft.zpos);
			a.scale(0.4, 0.4, 0.4);
			a.render(cursor3dLeft);
			a.popMat();
		}*/

		//currentX = cubes[currentCube]->xpos;
		//currentY = cubes[currentCube]->ypos;
		//currentZ = cubes[currentCube]->zpos;
		if(menuOpen == false)
		{
			internalX = hand1.xpos - currentX;
			internalY = hand1.ypos - currentY;
			internalZ = hand1.depth - currentZ;
			float cubeSize = 25;
			int closest = -1;
			float dist = 1000;

			bool notMoving = false;
			if (cubes[currentCube]->getRigidBody()->getLinearVelocity().norm() < 0.5){ //can only choose another cube when current one isn't moving
				notMoving = true;
			}

			if (sqrt((internalX*internalX) + (internalY*internalY) + (internalZ*internalZ)*2) > cubeSize && rightPose != 2 && notMoving) {
				btVector3 vec1(hand1.xpos, hand1.ypos, hand1.depth);
				btVector3 vec2(currentX, currentY, currentZ);
				btVector3 cubeToHand = (vec2 - vec1);// .normalize();

				float cosineSimilarity = 0.9;
				int nextCube = -1;
				int nextCube2 = -1;
				btVector3 currentCubeVec(cubes[currentCube]->xpos, cubes[currentCube]->ypos, -cubes[currentCube]->zpos);
				btVector3 otherCubeVec;
				btVector3 cubeToCube;
				for (int c = 0; c < cubes.size(); c++) {
					if (c != currentCube) {
						otherCubeVec = btVector3(cubes[c]->xpos, cubes[c]->ypos, -cubes[c]->zpos);
						cubeToCube = (otherCubeVec - currentCubeVec);

						float distComparison = (cubeToCube - cubeToHand / 10).norm(); //favour the closer cubes
						float cosineComparison = cubeToHand.dot(cubeToCube) / (cubeToHand.norm()*cubeToCube.norm());
						if (distComparison < dist && cosineComparison > 0.25) {
							dist = distComparison;
							nextCube2 = c;
						}
					}
				}

				if (nextCube2 != -1) {
					currentCube = nextCube2;
					currentX = hand1.xpos;
					currentY = hand1.ypos;
					currentZ = hand1.depth;
				}
			}

			if (rightPose == 2) {
				a.applyForce(*cubes[currentCube], GetTickCount() - stepTime, (prevHand1.xpos - hand1.xpos) * 1000, (prevHand1.ypos - hand1.ypos) * 1000, (hand1.depth - prevHand1.depth) * 1000);
			}
			else if (rightPoses.back() == 2 && rightPose != 2 && abs(hand1.depth - prevHand1.depth) < 15 && abs(hand1.xpos - prevHand1.xpos) + abs(hand1.ypos - prevHand1.ypos) < 10) { //15, 10 are original because can fly off a little if opening hand due to movement of central point and change of depth
				cubes[currentCube]->getRigidBody()->setLinearVelocity(btVector3(0, 0, 0));
				cubes[currentCube]->getRigidBody()->setAngularVelocity(btVector3(0, 0, 0));
			}
		

			if (twoHands) {
				internalXLeft = hand2.xpos - currentXLeft;
				internalYLeft = hand2.ypos - currentYLeft;
				internalZLeft = hand2.depth - currentZLeft;
				float cubeSize = 25;
				int closest = -1;
				float dist = 1000;

				bool notMoving = false;
				if (cubes[currentCubeLeft]->getRigidBody()->getLinearVelocity().norm() < 0.5) { //can only choose another cube when current one isn't moving
					notMoving = true;
				}

				if (sqrt(internalXLeft*internalXLeft + internalYLeft*internalYLeft + internalZLeft*internalZLeft) > cubeSize && leftPose != 2 && notMoving) {
					btVector3 vec1(hand2.xpos, hand2.ypos, hand2.depth);
					btVector3 vec2(currentXLeft, currentYLeft, currentZLeft);
					btVector3 cubeToHand = (vec2 - vec1);// .normalize();

					float cosineSimilarity = 0.9;
					int nextCube = -1;
					int nextCube2 = -1;
					btVector3 currentCubeVec(cubes[currentCubeLeft]->xpos, cubes[currentCubeLeft]->ypos, -cubes[currentCubeLeft]->zpos);
					btVector3 otherCubeVec;
					btVector3 cubeToCube;
					for (int c = 0; c < cubes.size(); c++) {
						if (c != currentCubeLeft) {
							otherCubeVec = btVector3(cubes[c]->xpos, cubes[c]->ypos, -cubes[c]->zpos);
							cubeToCube = (otherCubeVec - currentCubeVec);

							float distComparison = (cubeToCube - cubeToHand / 10).norm(); //favour the closer cubes
							float cosineComparison = cubeToHand.dot(cubeToCube) / (cubeToHand.norm()*cubeToCube.norm());
							if (distComparison < dist && cosineComparison > 0.25) {
								dist = distComparison;
								nextCube2 = c;
							}
						}
					}

					if (nextCube2 != -1) {
						currentCubeLeft = nextCube2;
						currentXLeft = hand2.xpos;
						currentYLeft = hand2.ypos;
						currentZLeft = hand2.depth;
					}
				}

				if (leftPose == 2) {
					a.applyForce(*cubes[currentCubeLeft], GetTickCount() - stepTime, (prevHand2.xpos - hand2.xpos) * 1000, (prevHand2.ypos - hand2.ypos) * 1000, (hand2.depth - prevHand2.depth) * 1000);
				}
				else if (leftPoses.back() == 2 && leftPose != 2 && abs(hand2.depth - prevHand2.depth) < 15 && abs(hand2.xpos - prevHand2.xpos) + abs(hand2.ypos - prevHand2.ypos) < 10) { //15, 10 are original because can fly off a little if opening hand due to movement of central point and change of depth
					cubes[currentCubeLeft]->getRigidBody()->setLinearVelocity(btVector3(0, 0, 0));
					cubes[currentCubeLeft]->getRigidBody()->setAngularVelocity(btVector3(0, 0, 0));
				}
			}
		}


		for (int c = 0; c < cubes.size(); c++) {
			Cube* cube = cubes[c];
			if (currentCube == currentCubeLeft && currentCube == c) {
				if (rightPose == 2 && leftPose == 2) {
					a.applyNegativeGravity(*cube); //Since anti gravity force is applied for each hand, this negates the effect of doublel anti-gravity
					float sizex = cube->getRigidBody()->getCenterOfMassPosition()[0];
					float sizey = cube->getRigidBody()->getCenterOfMassPosition()[1];
					float sizez = cube->getRigidBody()->getCenterOfMassPosition()[2];
					float distance = abs((float)hand1.xpos - (float)hand2.xpos) / 200; //moving away in x direction grows cube
					if (sizex + distance > 15) {
						a.setPosition(*cubes[currentCube], xRoomSize - distance - 0.1, sizey, sizez);
					}
					if (sizex - distance < -15) {
						a.setPosition(*cubes[currentCube], -xRoomSize + distance + 0.1, sizey, sizez);
					}
					if (sizey - distance < 0) {
						a.setPosition(*cubes[currentCube], sizex, 1 + distance, sizez);
					}
					if (sizez + distance > 10) {
						a.setPosition(*cubes[currentCube], sizex, sizey, zRoomSizeBack - distance - 0.1);
					}
					if (sizez - distance < -15) {
						a.setPosition(*cubes[currentCube], sizex, sizey, -zRoomSizeFront + distance + 0.1);
					}
					cube->setScaleFactor(distance);
					cubes[currentCube] = a.replace(*cube, cube->getRigidBody()->getCenterOfMassPosition()[0], cube->getRigidBody()->getCenterOfMassPosition()[1], cube->getRigidBody()->getCenterOfMassPosition()[2], cube->getMass(), 1.0*cube->getScaleFactor(), 1.0*cube->getScaleFactor(), 1.0*cube->getScaleFactor());
				}
				cube->setColour(1, 1, 1);
			}
			else if (c != currentCube && c != currentCubeLeft && (prevCube == c || prevCubeLeft == c)) {
				cube->setColour(1, 1, 0);
			}
			else if (c == currentCube && (prevCube == c || prevCubeLeft == c)) {
				cube->setColour(0.9, 0.1, 0.1);
			}
			else if ((prevCube == c || prevCubeLeft == c)) {
				cube->setColour(0.1, 0.1, 0.9);
			}
			cube->setXYZ(cube->getRigidBody()->getCenterOfMassPosition()[0], cube->getRigidBody()->getCenterOfMassPosition()[1], cube->getRigidBody()->getCenterOfMassPosition()[2]);
			if ((prevCube == c || prevCubeLeft == c)){
				cube->doBuffers();
			}
			a.pushMat();
			a.updateCube(*cube);
			a.scale(cube->getScaleFactor(), cube->getScaleFactor(), cube->getScaleFactor()); //If double cube size, scale double as well, but after updated cube
			a.render(*cube);
			a.popMat();
		}

	
		

		Mat noMenuImg;
		noMenuImg.create(a.getHeight(), a.getWidth(), CV_8UC3);
		glReadPixels(0, 0, noMenuImg.cols, noMenuImg.rows, GL_BGR, GL_UNSIGNED_BYTE, noMenuImg.data);
		
		Mat justMenu;
		justMenu.create(a.getHeight(), a.getWidth(), CV_8UC3);

		if (menuOpen == true) { //stay open
			a.menuLightingOff();
			mat4 p = a.getP(); //orthographic projection for menu
			double width = a.getWidth();
			double height = a.getHeight();
			float cursorx = -(float)fingertip1.x;
			float cursory = -(float)fingertip1.y;
			float cursorxhand = -(float)hand1.xpos;
			float cursoryhand = -(float)hand1.ypos;
			float maxy = cdepth.getHeight();
			float maxx = cdepth.getWidth();
			float globalFingerX = (cursorxhand + maxx / 2 + (cursorx - cursorxhand) / 5) / 1.5;
			float globalFingerY = (cursoryhand + maxy / 2 + (cursory - cursoryhand) / 5) / 2.0;
			a.setP(ortho(-width*0.1, width*0.1, -height*0.1, height*0.1, 0.0, 50.0)); //must be doubles or floats, smaller values give bigger square
			glDisable(GL_DEPTH_TEST);			

			//background
			a.pushMat();
				a.scale(width*0.01 - 0.5, height*0.01 - 0.5, 0);
				a.render(menuBackground);
			a.popMat();

			glReadPixels(0, 0, justMenu.cols, justMenu.rows, GL_BGR, GL_UNSIGNED_BYTE, justMenu.data);
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);//Set the background colour
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //these two could be quite computationally intensive


			//trackbar 1
			a.pushMat();
				a.updatePosition(tbar1); //go from bullet to opengl
				a.scale(5, 0.5, 0);
				a.render(tbar1); //draw
			a.popMat();

			//trackbar 2
			a.pushMat();
				a.updatePosition(tbar2); //go from bullet to opengl
				a.scale(5, 0.5, 0);
				a.render(tbar2); //draw
			a.popMat();
					
			//position of cursor
			a.setPosition(cursor, (float)(cursorxhand + maxx / 2), (float)(cursoryhand + maxy / 2), 0);

			//move slider1
			a.pushMat();
				if (rightPose == 3 && cursor.within(slider1, 15, 10) && cursor.within(tbar1,50,100)) { //move slider with finger. 100 is big enough to make sure whole slider covered, but is really irrelevant as this is just checking if slider is within x range of trackbar.
					a.setPosition(slider1, cursor.getRigidBody()->getCenterOfMassPosition()[0], slider1.getRigidBody()->getCenterOfMassPosition()[1], 0);
					slider1.setValue(abs((slider1.getRigidBody()->getCenterOfMassPosition()[0])-60)/4);
					float sizex = cubes[currentCube]->getRigidBody()->getCenterOfMassPosition()[0];
					float sizey = cubes[currentCube]->getRigidBody()->getCenterOfMassPosition()[1];
					float sizez = cubes[currentCube]->getRigidBody()->getCenterOfMassPosition()[2];
					float newDist = 1.0f*slider1.getValue() / 10.0; //distance between centre of cube and edge of cube
					if (sizex + newDist > 15) {
						a.setPosition(*cubes[currentCube], xRoomSize - newDist-0.1, sizey, sizez);
					}
					if (sizex - newDist < -15) {
						a.setPosition(*cubes[currentCube], -xRoomSize + newDist+0.1, sizey, sizez);
					}
					if (sizey - newDist < 0) {
						a.setPosition(*cubes[currentCube], sizex, 1 + newDist, sizez);
					}
					if (sizez + newDist > 10) {
						a.setPosition(*cubes[currentCube], sizex, sizey, zRoomSizeBack - newDist-0.1);
					}
					if (sizez - newDist < -15) {
						a.setPosition(*cubes[currentCube], sizex, sizey, -zRoomSizeFront + newDist+0.1);
					}
					//btCollisionShape* shape = new btBoxShape(btVector3(1.0f*slider1.getValue()/10.0, 1.0f * slider1.getValue()/10.0, 1.0f * slider1.getValue()/10.0));
					//cubes[mostRecentCube]->getRigidBody()->setCollisionShape(shape);
					//a.setPosition(cube1, 0, 20, 0);
					cubes[currentCube]->setScaleFactor(slider1.getValue() / 10.0);
					//cube1.getRigidBody()->setMassProps(5.0f, btVector3(0.0, 0.0, 0.0));
					cubes[currentCube] = a.replace(*cubes[currentCube], cubes[currentCube]->getRigidBody()->getCenterOfMassPosition()[0], cubes[currentCube]->getRigidBody()->getCenterOfMassPosition()[1], cubes[currentCube]->getRigidBody()->getCenterOfMassPosition()[2], cubes[currentCube]->getMass() , 1.0*cubes[currentCube]->getScaleFactor(), 1.0*cubes[currentCube]->getScaleFactor(), 1.0*cubes[currentCube]->getScaleFactor());
					//cubes[mostRecentCube]->setScaleFactor(slider1.getValue() / 10.0);
					cubes[currentCube]->setColour(1, 1, 0);
					cubes[currentCube]->doBuffers();
				}
				a.updatePosition(slider1);
				a.render(slider1);
			a.popMat();

			//move slider2
			a.pushMat();
				if (rightPose == 3 && cursor.within(slider2, 15, 10) && cursor.within(tbar2, 50, 100)) { //move slider with finger. 100 is big enough to make sure whole slider covered, but is really irrelevant as this is just checking if slider is within x range of trackbar.
					a.setPosition(slider2, cursor.getRigidBody()->getCenterOfMassPosition()[0], slider2.getRigidBody()->getCenterOfMassPosition()[1], 0);
					slider2.setValue((abs(slider2.getRigidBody()->getCenterOfMassPosition()[0] - 60)) / 2);
					float newMass = 1.0f*slider2.getValue() / 25.0;
					if (newMass > 1) {
						newMass = newMass*newMass*newMass*newMass;
					}
					//Cube* oldCube = (cubes[mostRecentCube]);
					cubes[currentCube] = a.replace(*cubes[currentCube], cubes[currentCube]->getRigidBody()->getCenterOfMassPosition()[0], cubes[currentCube]->getRigidBody()->getCenterOfMassPosition()[1], cubes[currentCube]->getRigidBody()->getCenterOfMassPosition()[2], newMass, 1.0*cubes[currentCube]->getScaleFactor(), 1.0*cubes[currentCube]->getScaleFactor(), 1.0*cubes[currentCube]->getScaleFactor());
					cubes[currentCube]->setColour(1, 1, 0);
					cubes[currentCube]->doBuffers();
					//cubes[mostRecentCube]->getRigidBody()->setMassProps(newMass, cubes[mostRecentCube]->getRigidBody()->getLocalInertia());
					//cubes[mostRecentCube]->setMass(newMass);
					//cout << cubes[mostRecentCube]->getMass() << endl;
					//Cube
					//a.setPosition(cube1, 0, 20, 0);
				}
				a.updatePosition(slider2);
				a.render(slider2);
			a.popMat();

			if (rightPose == 3) {
				cursor.setColour(1, 0, 0);
				cursor.doBuffers();
			}
			else {
				cursor.setColour(0.5, 0.5, 0.5);
				cursor.doBuffers();
			}
			a.pushMat();
				a.updatePosition(cursor);
				a.scale(0.2, 0.2, 0.2);
				a.render(cursor);
			a.popMat();

			glEnable(GL_LIGHTING);
			glEnable(GL_DEPTH_TEST);
			a.setP(p);

			menuOpen = true;
			menuCounter++;
		}
		else {
			menuClosedCounter++;
			a.lightingOn();
			glReadPixels(0, 0, justMenu.cols, justMenu.rows, GL_BGR, GL_UNSIGNED_BYTE, justMenu.data);
		}
		
		if (a.menuCloseGesture(rightPoses)){// && menuCounter > frameCheck){ //go from open to closed
			menuOpen = false;
			menuCounter = 0;
		}

		if (a.menuOpenGesture(rightPoses) && menuOpen == false && menuClosedCounter > frameCheck) { //open from closed
			menuOpen = true;
			menuClosedCounter = 0;
		}
		
		Mat menuImg(a.getHeight(), a.getWidth(), CV_8UC3); //just the sliders and cursor
		glReadPixels(0, 0, menuImg.cols, menuImg.rows, GL_BGR, GL_UNSIGNED_BYTE, menuImg.data);

		Mat translucentbackground(a.getHeight(), a.getWidth(), CV_8UC3);

		addWeighted(noMenuImg, 0.5, justMenu, 0.5, 0, translucentbackground);
		//addWeighted(translucentbackground, 1, menuImg, 0.5, 0, translucentbackground);
		if (menuOpen) {
			addWeighted(translucentbackground, 1, menuImg, -1, 0, translucentbackground);
		}

		a.setOglimg(translucentbackground);

		a.postprocessing();

		globalCounter++;
		prevPrevHand1 = prevHand1;
		prevHand1 = hand1;
		prevCursor3d.setXYZ(cursor3dRight.xpos, cursor3dRight.ypos, cursor3dRight.zpos);
		rightPoses.pop_front();
		rightPoses.push_back(rightPose);
		prevCube = currentCube;
		if (twoHands) {
			leftPoses.pop_front();
			leftPoses.push_back(leftPose);
			prevHand2 = hand2;
			prevCubeLeft = currentCubeLeft;
		}
		
		firstFrame = false;

		//display output
		a.outputImage(outputImageName, cdepth.Draw(2));

		if (menuOpen) {
			a.stepMenu(GetTickCount() - stepTime);
		}
		else {
			a.step(GetTickCount() - stepTime);
		}

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
			//a.setPosition(stillCube, 0, 1, 0);
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



