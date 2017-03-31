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

bool firstFrame = true;
bool run = true; //whether program continues to run
bool draw = true; //whether to output the opencv processed images
bool twoHands = true; //whether accomodates for 2 hands
int checkstep = 1;
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

//to do:
//flip left hand hand image
//make stuff work with 2nd cursor


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

	a.makeCube(0, 10, -5, &cubes, &cubeState, &leftCubeState);
	a.makeCube(0, 5, -5, &cubes, &cubeState, &leftCubeState);
	a.makeCube(0, 1, -5, &cubes, &cubeState, &leftCubeState);
	a.makeCube(2, 10, -2, &cubes, &cubeState, &leftCubeState);
	a.makeCube(2, 5, -2, &cubes, &cubeState, &leftCubeState);
	a.makeCube(2, 1, -2, &cubes, &cubeState, &leftCubeState);
	a.makeCube(-2, 10, -2, &cubes, &cubeState, &leftCubeState);
	a.makeCube(-2, 5, -2, &cubes, &cubeState, &leftCubeState);
	a.makeCube(-2, 1, -2, &cubes, &cubeState, &leftCubeState);
	a.makeCube(-4, 10, 2, &cubes, &cubeState, &leftCubeState);
	a.makeCube(-4, 5, 2, &cubes, &cubeState, &leftCubeState);
	a.makeCube(-4, 1, 2, &cubes, &cubeState, &leftCubeState);
	a.makeCube(4, 10, 2, &cubes, &cubeState, &leftCubeState);
	a.makeCube(4, 5, 2, &cubes, &cubeState, &leftCubeState);
	a.makeCube(4, 1, 2, &cubes, &cubeState, &leftCubeState);
	a.makeCube(0, 1, -10, &cubes, &cubeState, &leftCubeState);
	a.makeCube(3, 1, -10, &cubes, &cubeState, &leftCubeState);
	a.makeCube(-3, 1, -10, &cubes, &cubeState, &leftCubeState);

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

	vector<Point> leftHandHull;
	Point fingertip1;

	vector<Point> rightHandHull;
	Point fingertip2;

	int leftPose;
	int rightPose;

	while (run && a.continueProcessing())
	{
		stepTime = GetTickCount();

		if (firstFrame != true) {
			cdepth.Update(); //Fills in depthArr
			cdepth.FindHands(false, checkstep, twoHands); //Find the position of the hand(s)
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
		if (menuOpen == false && rightPose == 3 && rightPoses[rightPoses.size() - 5] == 3) {
			offsetx = -((float)cdepth.getWidth() / 2.0 - (float)hand1.xpos) / sensitivityScale; //the distance from the original position, used when resetting position
			offsety = -((float)cdepth.getHeight() / 2.0 - (float)hand1.ypos) / sensitivityScale;
			offsetz = ((cdepth.getMaxDepth() - cdepth.getMinDepth()) / 2.0 - 80 - (float)hand1.depth + offsetz) / sensitivityScale;
		}
		cursor3dRight.setXYZ(((float)cdepth.getWidth() / 2.0 - (float)hand1.xpos) / sensitivityScale + offsetx, ((float)cdepth.getHeight() / 2.0 - (float)hand1.ypos + offsety) / sensitivityScale + offsety, -((cdepth.getMaxDepth() - cdepth.getMinDepth()) / 2.0 - 80 - (float)hand1.depth + offsetz) / (sensitivityScale/1.5) + offsetz);

		if (twoHands) {
			if (menuOpen == false && leftPose == 3 && leftPoses[leftPoses.size() - 5] == 3) {
				offsetx2 = -((float)cdepth.getWidth() / 2.0 - (float)hand2.xpos) / sensitivityScale;
				offsety2 = -((float)cdepth.getHeight() / 2.0 - (float)hand2.ypos) / sensitivityScale;
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
				float range = cube->getScaleFactor() + 0;

				//letting go of cube
				if (cubeState[c] == 1 && rightPose != 2 && rightPoses.back() == 2 && abs(hand1.depth - prevHand1.depth) < 15 && abs(hand1.xpos - prevHand1.xpos) + abs(hand1.ypos - prevHand1.ypos) < 10) { // because can fly off a little if opening hand due to movement of central point and change of depth
					cube->getRigidBody()->setLinearVelocity(btVector3(0, 0, 0));
					cube->getRigidBody()->setAngularVelocity(btVector3(0, 0, 0));
					cubeState[c] = 0; //not doing anything
				}
				else if (rightPose != 2) { //throwing cube
					cubeState[c] = 0;
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
					}
					else if (leftPose != 2) { //throwing cube
						leftCubeState[c] = 0;
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
					}
					else if ((cubeChosen == false && cursor3dRight.within(*cube, range, range, range)) || cubeState[c] == 1) {
						cube->setColour(1, 1, 1);
						cube->doBuffers();
						cubeChosen = true;
						mostRecentCube = c;
					}
					else {
						cube->setColour(1, 1, 0);
						cube->doBuffers();
					}
				}
				
				if (twoHands == false) {
					if ((cubeChosen == false && cursor3dRight.within(*cube, range, range, range)) || cubeState[c] == 1) {
						cube->setColour(1, 1, 1);
						cube->doBuffers();
						cubeChosen = true;
						mostRecentCube = c;
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
		}
		
		if (cubeChosen == false && menuOpen == false) { //cant go in earlier if since must check cubeChosen after all have been iterated through
			a.pushMat();
			a.translate(cursor3dRight.xpos, cursor3dRight.ypos, cursor3dRight.zpos);
			a.scale(0.4, 0.4, 0.4);
			a.render(cursor3dRight);
			a.popMat();
		}
		if (leftCubeChosen == false && menuOpen == false && twoHands) {
			a.pushMat();
			a.translate(cursor3dLeft.xpos, cursor3dLeft.ypos, cursor3dLeft.zpos);
			a.scale(0.4, 0.4, 0.4);
			a.render(cursor3dLeft);
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
					float sizex = cubes[mostRecentCube]->getRigidBody()->getCenterOfMassPosition()[0];
					float sizey = cubes[mostRecentCube]->getRigidBody()->getCenterOfMassPosition()[1];
					float sizez = cubes[mostRecentCube]->getRigidBody()->getCenterOfMassPosition()[2];
					float newDist = 1.0f*slider1.getValue() / 10.0; //distance between centre of cube and edge of cube
					if (sizex + newDist > 15) {
						a.setPosition(*cubes[mostRecentCube], xRoomSize - newDist-0.1, sizey, sizez); 
					}
					if (sizex - newDist < -15) {
						a.setPosition(*cubes[mostRecentCube], -xRoomSize + newDist+0.1, sizey, sizez);
					}
					if (sizey - newDist < 0) {
						a.setPosition(*cubes[mostRecentCube], sizex, 1 + newDist, sizez);
					}
					if (sizez + newDist > 10) {
						a.setPosition(*cubes[mostRecentCube], sizex, sizey, zRoomSizeBack - newDist-0.1);
					}
					if (sizez - newDist < -15) {
						a.setPosition(*cubes[mostRecentCube], sizex, sizey, -zRoomSizeFront + newDist+0.1);
					}
					btCollisionShape* shape = new btBoxShape(btVector3(1.0f*slider1.getValue()/10.0, 1.0f * slider1.getValue()/10.0, 1.0f * slider1.getValue()/10.0));
					cubes[mostRecentCube]->getRigidBody()->setCollisionShape(shape);
					//a.setPosition(cube1, 0, 20, 0);
					cubes[mostRecentCube]->setScaleFactor(slider1.getValue() / 10.0);
					//cube1.getRigidBody()->setMassProps(5.0f, btVector3(0.0, 0.0, 0.0));
				
				}
				a.updatePosition(slider1);
				a.render(slider1);
			a.popMat();

			//move slider2
			a.pushMat();
				if (rightPose == 3 && cursor.within(slider2, 15, 10) && cursor.within(tbar2, 50, 100)) { //move slider with finger. 100 is big enough to make sure whole slider covered, but is really irrelevant as this is just checking if slider is within x range of trackbar.
					a.setPosition(slider2, cursor.getRigidBody()->getCenterOfMassPosition()[0], slider2.getRigidBody()->getCenterOfMassPosition()[1], 0);
					slider2.setValue((abs(slider2.getRigidBody()->getCenterOfMassPosition()[0]) - 20) / 2);

					cubes[mostRecentCube]->getRigidBody()->setMassProps(1.0f*slider2.getValue() / 10.0, cubes[mostRecentCube]->getRigidBody()->getLocalInertia());
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
		
		if(a.menuOpenGesture(rightPoses) && menuCounter > frameCheck){ //go from open to closed
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
		prevHand1 = hand1;
		prevCursor3d.setXYZ(cursor3dRight.xpos, cursor3dRight.ypos, cursor3dRight.zpos);
		rightPoses.pop_front();
		rightPoses.push_back(rightPose);
		if (twoHands) {
			leftPoses.pop_front();
			leftPoses.push_back(leftPose);
			prevHand2 = hand2;
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



