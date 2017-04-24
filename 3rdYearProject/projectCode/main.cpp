#pragma once

#include "stdafx.h"
#include <strsafe.h>
#include "resource.h"
#include "ml.h"
#include "header.h"
#include "DepthBasics2.h"
#include "environment.h"
#include <fstream>
#include <deque>      
#include <vector> 
#include <fstream>
#include <cmath>


using namespace std;
using namespace cv;
using namespace shark;
using namespace glm;

bool firstFrame = true;
bool run = true; //whether program continues to run
bool draw = true; //whether to output the opencv processed images
int checkstep = 2;
bool freemove = false; //whether can move freely around the environment
int ksize = 45; //kernel size in Gaussian blur for hand detection
int sigma = 100; //sigma size for gaussian blur
int checkSize = 50; //aread around current hand centre to check for next hand
int fps = 1000; //maximum possible, not actual fps value. Used in cvshow(1/fps)
int maxDepth = 1100; //maximum distance away, in mm
//int movement = 0; //
unsigned long stepTime = GetTickCount(); //A Windows only function, gives time in milliseconds
int counter = 0; //used for testing
int globalCounter = 0; //used for testing
int numSavedPoses = 15; //Number of previous poses saved at any one time
int frameCheck = numSavedPoses; //number of frames to check when openinf menu
string outputImageName = "OpenGL image";


//fstream output; 


pixel prevHand1; //hand in previous frame
pixel prevPrevHand1; //2 hands ago
pixel prevHand2; //previous 2nd hand
deque<int> rightPoses; //poses made by right hand
deque<int> leftPoses; //poses made by left hand
bool menuOpen = false; //if menu is open
int menuCounter = 0; //number of frames manu has been open for
int menuClosedCounter = 0; //number of frames menu closed for
bool grabbing = false; //if a cube is currently being grabbed
vector<Cube*> cubes; //holds all the cubes in the environment
vector<int> cubeState; //state of the cubes. 0 = no interaction, 1 = currently active cube
vector<int> leftCubeState; //state of cubes in the left hand
int mostRecentCube = 0; //most recently accessed cube
float offsetx = 0; //used with cursor
float offsety = 0;
float offsetz = 0;
float offsetx2 = 0;
float offsety2 = 0;
float offsetz2 = 0;

//room dimensions
float xRoomSize = 15; 
float zRoomSizeBack = 10;
float zRoomSizeFront = 15;

//initial cube attrubites
float startMass = 5;
float startxdim = 1.0;
float startydim = 1.0;
float startzdim = 1.0;

float axisSize = 0.02; //used with cursor

//used to determine if hand has moved enough to select new cube
float currentX = 0;
float currentY = 0;
float currentZ = 0;

float internalX = 0;
float internalY = 0;
float internalZ = 0;

int currentCube = 7; //currently seleced right cube

float currentXLeft = 0;
float currentYLeft = 0;
float currentZLeft = 0;

float internalXLeft = 0;
float internalYLeft = 0;
float internalZLeft = 0;

int currentCubeLeft = 4; //currenly selected left cube

int prevCube = -1;
int prevCubeLeft = -1;

bool twoHands = false; //whether accomodates for 2 hands

bool updateRegularly = true; //whether to search whole image for hand

void main()
{
	namedWindow(outputImageName, WINDOW_AUTOSIZE); //create opencv window - this is how the image is displayed

	//output.open("data.csv", ios::app);//fstream::out);

	CDepthBasics cdepth; //create the object that finds and tracks hand
	ML ml; //create object that eprforms pose classification
	App a; //create object that deals with interactive application

	a.setCamera(0, 5, -25); //set camera position
	a.setupView(); //set up the opengl
	a.setupPhysics(); //set up Bullet physics

	cdepth.SetUp(twoHands, ksize, sigma, checkSize, draw, checkstep, maxDepth); //set up all that is needed to track the hands

	vector<vector<int > > startPositions = { //the positions of the boxes. +x is to the left, +y is up, +z is away from the user
		{0, 10 ,-5},
		{0, 5, -5},
		{0,1,-5},
		{2,10,-2},
		{2,5,-2},
		{2,1,-2},
		{-2,10,-2},
		{ -2, 5, -2 },
		{ -2, 1, -2 },
		{ -4, 10, 2 },
		{ -4, 5, 2 },
		{ -4, 1, 2 },
		{ 4, 10, 2 },
		{ 4, 5, 2 },
		{ 4, 1, 2 },
		{ 0, 1, -10 },
		{ 3, 1, -10 },
		{ -3, 1, -10 },
	};

	for (int i = 0; i < startPositions.size(); i++) { //initialise the cubes
		a.makeCube(startPositions[i][0], startPositions[i][1], startPositions[i][2], startMass, startxdim, startydim, startzdim, &cubes, &cubeState, &leftCubeState);
	}
	
	Cube xaxis; //used with cursor

	Menu2D menuBackground; //create 2D background

	Trackbar tbar1; //create top trackbar
	a.addToMenuWorld(tbar1.setUpPhysics(btVector3(0, 30, 2)));//add to physics worls and set position
	Trackbar tbar2;
	a.addToMenuWorld(tbar2.setUpPhysics(btVector3(0, -10, 2)));//70

	Cursor cursor; //3D cursor, no longer used
	a.addToMenuWorld(cursor.setUpPhysics(btVector3(0, 5, 2)));

	Cursor fingertipCursor; //cursor for 3d menu
	fingertipCursor.setColour(1, 0, 0); //set colour
	fingertipCursor.doBuffers(); //update colour buffers

	//3d cursors, no longer used
	Cube cursor3dRight;
	cursor3dRight.setColour(0.3, 0.8, 1);
	cursor3dRight.doBuffers();

	Cube cursor3dLeft;
	cursor3dLeft.setColour(1, 0.8, 0.3);
	cursor3dLeft.doBuffers();
	
	Cube prevCursor3d;

	Slider slider1; //create sliders
	a.addToMenuWorld(slider1.setUpPhysics(btVector3(20, 10, 0)));//25
	Slider slider2;
	a.addToMenuWorld(slider2.setUpPhysics(btVector3(0, -30, 0)));//25

	Plane plane1; //create floor
	a.addToWorld(plane1.setUpPhysics(btVector3(0,1,0), btVector3(0,-1,0)));///y=-1
	plane1.setColour(0.5, 0, 0.5);
	plane1.doBuffers();

	Plane rightWall; //create right wall
	a.addToWorld(rightWall.setUpPhysics(btVector3(1, 0, 0), btVector3(-xRoomSize, 0, 0)));
	rightWall.setColour(0.5, 0.5, 0);
	rightWall.doBuffers();
	
	Plane leftWall; //create left wall
	a.addToWorld(leftWall.setUpPhysics(btVector3(-1, 0, 0), btVector3(xRoomSize, 0, 0)));
	leftWall.setColour(0.5, 0.5, 0);
	leftWall.doBuffers();
	
	Plane backWall; //create back wall
	a.addToWorld(backWall.setUpPhysics(btVector3(0, 0, -1), btVector3(0, 0, zRoomSizeBack)));
	backWall.setColour(0.35, 0.6, 0.25);
	backWall.doBuffers();

	Plane closeWall; //create wall close to user. Invisible.
	a.addToWorld(closeWall.setUpPhysics(btVector3(0, 0, 1), btVector3(0, 0, -zRoomSizeFront)));
	
	for (int i = 0; i < numSavedPoses; i++) //initialise pose arrays
	{
		rightPoses.push_back(0);
		leftPoses.push_back(0);
	}

	//get hand locations
	pixel hand1 = cdepth.getHand1();
	pixel hand2 = cdepth.getHand2();
	pair<vector<Point>, Point> leftHand; //stores hand info
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

	//position of hands
	currentX = hand1.xpos;
	currentY = hand1.ypos;
	currentZ = hand1.depth;
	currentXLeft = hand2.xpos;
	currentYLeft = hand2.ypos;
	currentZLeft = hand2.depth;
	if (twoHands == false) {
		currentCubeLeft = -1;
	}

	vector<Point> leftHandHull; //hand info
	Point fingertip1;

	vector<Point> rightHandHull;
	Point fingertip2;

	int leftPose; //current pose being made
	int rightPose;


	while (run && a.continueProcessing()) //continue to run. Each loop is a new frame. Press a key that isnt z,x,c,o,p to exit
	{
		//cout << counter << endl;
		if (updateRegularly) {
			counter = (counter + 1) % 20;
		}
		else {
			counter = 1;
		}
		
		globalCounter++;
		
		stepTime = GetTickCount(); //used for measuring distance for forces on cube

		if (firstFrame != true) {
			cdepth.Update(); //Fills in depthArr
			if (counter == 0) {
				cdepth.FindHands(true, checkstep, twoHands); //Find the position of the hand(s), but search whole image
			}
			else {
				cdepth.FindHands(false, checkstep, twoHands); //Find the position of the hand(s), but just local to current position
			}
		}

		if (twoHands) {
			if (firstFrame != true) {
				hand1 = cdepth.getHand1();
				hand2 = cdepth.getHand2();

				rightHand = cdepth.ProcessHand(hand1, draw, 1); //Get the information used for pose identification
				leftHand = cdepth.ProcessHand(hand2, draw, 2); //Get the information used for pose identification
			}
			if (globalCounter == 0) { //initialise the previous hand
				prevHand1 = hand1;
			}

			leftHandHull = leftHand.first; //get hull info
			fingertip1 = leftHand.second; //get fingertip info

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
			cdepth.DrawRightClassification(rightPose); //draw the classification info
			if (twoHands) {
				cdepth.DrawLeftClassification(leftPose);
			}
			//cdepth.Draw(2);
		}

		a.preprocessing(freemove); //preprocessig for opengl

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

		a.pushMat(); 
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
		
		
		if(menuOpen == false)
		{
			internalX = hand1.xpos - currentX; //find distance hand has moved from centre of current cube
			internalY = hand1.ypos - currentY; //if above certain distance, then assumes hand has moved "out of " the cube, so choose the nearest next cube
			internalZ = hand1.depth - currentZ;
			float cubeSize = 25; //distance to move to be outide the cube
			int closest = -1; //initially closest cube. -1 since can not be a suitabe closest
			float dist = 1000; //initialise distance value

			bool notMoving = false; //if cube is not moving
			if (cubes[currentCube]->getRigidBody()->getLinearVelocity().norm() < 0.5){ //can only choose another cube when current one isn't moving
				notMoving = true;
			}

			if (sqrt((internalX*internalX) + (internalY*internalY) + (internalZ*internalZ)*2) > cubeSize && rightPose != 2 && notMoving) { //if hand has moved outside of the cube, is not grabbing, and the cube is not moving
				btVector3 vec1(hand1.xpos, hand1.ypos, hand1.depth); //vector from origin to the hand
				btVector3 vec2(currentX, currentY, currentZ); //vector from origin to the centre of the current cube
				btVector3 cubeToHand = (vec2 - vec1);// .normalize(); //get vectore from hand to cube
				//these vectors are in terms of depth image coordinates, and as such are not directly comparable to the physics engine coordinates 

				float cosineSimilarity = 0.9; //threshold for cosine similarity, a measure of how similar vector directions are
				//cosine similarity is a value from -1 to 1. 1 means exactly the same direction, -1 is exact opposite direction
				int nextCube = -1;
				int nextCube2 = -1;
				btVector3 currentCubeVec(cubes[currentCube]->xpos, cubes[currentCube]->ypos, -cubes[currentCube]->zpos); //vector from origin to current cube
				btVector3 otherCubeVec; //holds vector from origin to current potential next cube
				btVector3 cubeToCube;
				for (int c = 0; c < cubes.size(); c++) { //go through the cubes
					if (c != currentCube) {
						otherCubeVec = btVector3(cubes[c]->xpos, cubes[c]->ypos, -cubes[c]->zpos); 
						cubeToCube = (otherCubeVec - currentCubeVec);

						float distComparison = (cubeToCube - cubeToHand / 10).norm(); //favour the closer cubes, divding by 10 makes physics engin and depth image coordinates reoughly comparable
						float cosineComparison = cubeToHand.dot(cubeToCube) / (cubeToHand.norm()*cubeToCube.norm()); //calculate cosine similarity
						if (distComparison < dist && cosineComparison > 0.25) { //if closer to cuerrent cube and roughly in the same direction, make this the current candidate for best cube
							dist = distComparison;
							nextCube2 = c;
						}
					}
				}

				if (nextCube2 != -1) { //assign next cube
					currentCube = nextCube2;
					currentX = hand1.xpos;
					currentY = hand1.ypos;
					currentZ = hand1.depth;
				}
			}

			if (rightPose == 2) { //if user grabs, apply a force to the cube. Force is calculated using SUVAT
				a.applyForce(*cubes[currentCube], GetTickCount() - stepTime, (prevHand1.xpos - hand1.xpos) * 1000, (prevHand1.ypos - hand1.ypos) * 1000, (hand1.depth - prevHand1.depth) * 1000);
			} //if user lets go. The < 15 and < 10 bits make sure not opening hand does not make cube fly away, since change of depth of opening fingers can be confused for cube moving forward quickly
			else if (rightPoses.back() == 2 && rightPose != 2 && abs(hand1.depth - prevHand1.depth) < 15 && abs(hand1.xpos - prevHand1.xpos) + abs(hand1.ypos - prevHand1.ypos) < 10) { //15, 10 are original because can fly off a little if opening hand due to movement of central point and change of depth
				cubes[currentCube]->getRigidBody()->setLinearVelocity(btVector3(0, 0, 0)); //make cube still
				cubes[currentCube]->getRigidBody()->setAngularVelocity(btVector3(0, 0, 0));
			}
		

			if (twoHands) { //do same for left hand
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

		//update state of all cubes
		for (int c = 0; c < cubes.size(); c++) {
			Cube* cube = cubes[c];
			if (currentCube == currentCubeLeft && currentCube == c) {
				if (rightPose == 2 && leftPose == 2) {
					a.applyNegativeGravity(*cube); //Since anti gravity force is applied for each hand, this negates the effect of doublel anti-gravity
					float sizex = cube->getRigidBody()->getCenterOfMassPosition()[0];
					float sizey = cube->getRigidBody()->getCenterOfMassPosition()[1];
					float sizez = cube->getRigidBody()->getCenterOfMassPosition()[2];
					float distance = abs((float)hand1.xpos - (float)hand2.xpos) / 200; //moving away in x direction grows cube
					if (sizex + distance > 15) { //move away from walls, as adding new rigid body that collides with walls can cause issues
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
					//remove current rigid body and add new one
					cubes[currentCube] = a.replace(*cube, cube->getRigidBody()->getCenterOfMassPosition()[0], cube->getRigidBody()->getCenterOfMassPosition()[1], cube->getRigidBody()->getCenterOfMassPosition()[2], cube->getMass(), 1.0*cube->getScaleFactor(), 1.0*cube->getScaleFactor(), 1.0*cube->getScaleFactor());
				}
				cube->setColour(1, 1, 1);
			}
			else if (c != currentCube && c != currentCubeLeft && (prevCube == c || prevCubeLeft == c)) {
				cube->setColour(1, 1, 0); //set colours
			}
			else if (c == currentCube && (prevCube == c || prevCubeLeft == c)) {
				cube->setColour(0.9, 0.1, 0.1);
			}
			else if ((prevCube == c || prevCubeLeft == c)) {
				cube->setColour(0.1, 0.1, 0.9);
			}
			//set position value
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
			/////

	
		
		//2d menu
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
			float cursorx = -(float)fingertip1.x; //cursor position info
			float cursory = -(float)fingertip1.y;
			float cursorxhand = -(float)hand1.xpos;
			float cursoryhand = -(float)hand1.ypos;
			float maxy = cdepth.getHeight();
			float maxx = cdepth.getWidth();
			float globalFingerX = (cursorxhand + maxx / 2 + (cursorx - cursorxhand) / 5) / 1.5; //cursor position in global coordinates, 0 as middle of screen not top left
			float globalFingerY = (cursoryhand + maxy / 2 + (cursory - cursoryhand) / 5) / 2.0;
			a.setP(ortho(-width*0.1, width*0.1, -height*0.1, height*0.1, 0.0, 50.0)); //must be doubles or floats, smaller values give bigger square
			glDisable(GL_DEPTH_TEST); //appeare 2D

			//background
			a.pushMat();
				a.scale(width*0.01 - 0.5, height*0.01 - 0.5, 0);
				a.render(menuBackground);
			a.popMat();

			glReadPixels(0, 0, justMenu.cols, justMenu.rows, GL_BGR, GL_UNSIGNED_BYTE, justMenu.data); //opengl data to opencv data
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
					slider1.setValue(abs((slider1.getRigidBody()->getCenterOfMassPosition()[0])-60)/4); //translate 2d position into value, so that is doesnt go below 0
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
					//increase size
					cubes[currentCube]->setScaleFactor(slider1.getValue() / 10.0);
					cubes[currentCube] = a.replace(*cubes[currentCube], cubes[currentCube]->getRigidBody()->getCenterOfMassPosition()[0], cubes[currentCube]->getRigidBody()->getCenterOfMassPosition()[1], cubes[currentCube]->getRigidBody()->getCenterOfMassPosition()[2], cubes[currentCube]->getMass() , 1.0*cubes[currentCube]->getScaleFactor(), 1.0*cubes[currentCube]->getScaleFactor(), 1.0*cubes[currentCube]->getScaleFactor());
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
						newMass = newMass*newMass*newMass*newMass; //amplify mass change
					}
					//increase mass
					cubes[currentCube] = a.replace(*cubes[currentCube], cubes[currentCube]->getRigidBody()->getCenterOfMassPosition()[0], cubes[currentCube]->getRigidBody()->getCenterOfMassPosition()[1], cubes[currentCube]->getRigidBody()->getCenterOfMassPosition()[2], newMass, 1.0*cubes[currentCube]->getScaleFactor(), 1.0*cubes[currentCube]->getScaleFactor(), 1.0*cubes[currentCube]->getScaleFactor());
					cubes[currentCube]->setColour(1, 1, 0);
					cubes[currentCube]->doBuffers();
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

			if (a.resetGesture(rightPoses)) { //open and close hand to reset cube positions
				for (int i = 0; i < cubes.size(); i++) {
					Cube* c = cubes[i];
					cubes[i] = a.replace(*c, startPositions[i][0], startPositions[i][1], startPositions[i][2], startMass, startxdim, startydim, startzdim);
					cubes[i]->setColour(1, 1, 0);
					cubes[i]->doBuffers();
					cubes[i]->setScaleFactor(1.0);
				}
				menuOpen = false;
				menuCounter = 0;
			}
		} //end of open menu
		else {
			menuClosedCounter++;
			a.lightingOn();
			glReadPixels(0, 0, justMenu.cols, justMenu.rows, GL_BGR, GL_UNSIGNED_BYTE, justMenu.data);
		}
		
		if (a.menuCloseGesture(rightPoses)){//go from open to closed
			menuOpen = false;
			menuCounter = 0;
		}

		if (a.menuOpenGesture(rightPoses) && menuOpen == false && menuClosedCounter > frameCheck) { //to open from closed
			menuOpen = true;
			menuClosedCounter = 0;
		}
		
		Mat menuImg(a.getHeight(), a.getWidth(), CV_8UC3); //just the sliders and cursor
		glReadPixels(0, 0, menuImg.cols, menuImg.rows, GL_BGR, GL_UNSIGNED_BYTE, menuImg.data); //get opengl data from when menu shown

		Mat translucentbackground(a.getHeight(), a.getWidth(), CV_8UC3); //stores the image with the translucent background

		addWeighted(noMenuImg, 0.5, justMenu, 0.5, 0, translucentbackground); //combine the images to appear translucent
		//addWeighted(translucentbackground, 1, menuImg, 0.5, 0, translucentbackground);
		if (menuOpen) {
			addWeighted(translucentbackground, 1, menuImg, -1, 0, translucentbackground);
		}

		a.setOglimg(translucentbackground); // the image to be shown

		a.postprocessing(); //opengl postprocessing

		globalCounter++;
		prevPrevHand1 = prevHand1; //assign hand so is ready for next frame
		prevHand1 = hand1;
		prevCursor3d.setXYZ(cursor3dRight.xpos, cursor3dRight.ypos, cursor3dRight.zpos);
		rightPoses.pop_front(); //update poses
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
		if (key == 'p') //make use 2 hands
		{
			//a.setPosition(stillCube, 0, 1, 0);
			twoHands = true;
			currentCubeLeft = 4;
		}
		else if (key == 'o') { //one hand only
			twoHands = false;
			currentCubeLeft = -1;
		}
		else if (key == 'q') //find hands
		{
			cdepth.FindHands(true, checkstep, twoHands);
		}
		else if (key != -1)
		{
			run = false;
		}
	}
	
}


//NO LONGER USED CURSOR CODE
///////
/*
if (menuOpen == false && rightPose == 3 && rightPoses[rightPoses.size() - 3] == 3 && rightPoses[rightPoses.size() - 5] == 3 && rightPoses[rightPoses.size() - 7] == 3) {
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
}

if (cubeChosen == false && menuOpen == false) { //cant go in earlier if since must check cubeChosen after all have been iterated through
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