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
deque<int> poses;
bool menuOpen = false;
int menuCounter = 0;
int menuClosedCounter = 0;
bool grabbing = false;
//Cube activeCube;
vector<Cube*> cubes;
vector<int> cubeState;
int mostRecentCube = 0;
float offsetx = 0;
float offsety = 0;
float offsetz = 0;

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
	cubes.push_back(&cube1);
	cubeState.push_back(0);

	Cube stillCube;
	a.addToWorld(stillCube.setUpPhysics(btVector3(0,1,0)));
	stillCube.setColour(1, 1, 0);
	stillCube.doBuffers();
	cubes.push_back(&stillCube);
	cubeState.push_back(0);


	Menu2D menuBackground;
	Trackbar tbar1;
	a.addToMenuWorld(tbar1.setUpPhysics(btVector3(-70, 30, 2)));
	Trackbar tbar2;
	a.addToMenuWorld(tbar2.setUpPhysics(btVector3(-70, -10, 2)));

	Cursor cursor;
	a.addToMenuWorld(cursor.setUpPhysics(btVector3(0, 5, 2)));
	Cursor fingertipCursor;
	fingertipCursor.setColour(1, 0, 0);
	fingertipCursor.doBuffers();

	Cube cursor3d;
	cursor3d.setColour(0.3, 0.8, 1);
	cursor3d.doBuffers();
	
	Cube prevCursor3d;

	Slider slider1;
	a.addToMenuWorld(slider1.setUpPhysics(btVector3(-25, 10, 0)));
	Slider slider2;
	a.addToMenuWorld(slider2.setUpPhysics(btVector3(-25, -30, 0)));
	/*slider1.setX(-a.getWidth()*0.05 + 0);//0
	slider1.setMinX(slider1.getXPos() + 20); //+ because left is positive in this instance of opengl.
	slider1.setMinX(slider1.getXPos() - 20);
	slider1.setY(a.getHeight()*0.05);*/

	Plane plane1;
	a.addToWorld(plane1.setUpPhysics(btVector3(0,1,0), btVector3(0,-1,0)));///y=-1
	plane1.setColour(0.5, 0, 0.5);
	plane1.doBuffers();

	Plane rightWall;
	a.addToWorld(rightWall.setUpPhysics(btVector3(1, 0, 0), btVector3(-15, 0, 0)));
	rightWall.setColour(0.5, 0.5, 0);
	rightWall.doBuffers();
	
	Plane leftWall;
	a.addToWorld(leftWall.setUpPhysics(btVector3(-1, 0, 0), btVector3(15, 0, 0)));
	leftWall.setColour(0.5, 0.5, 0);
	leftWall.doBuffers();
	
	Plane backWall;
	a.addToWorld(backWall.setUpPhysics(btVector3(0, 0, -1), btVector3(0, 0, 10)));
	backWall.setColour(0.35, 0.6, 0.25);
	backWall.doBuffers();

	Plane closeWall;
	a.addToWorld(closeWall.setUpPhysics(btVector3(0, 0, 1), btVector3(0, 0, -15)));

	a.setPosition(stillCube, 5, 15, 0);

	for (int i = 0; i < numSavedPoses; i++)
	{
		poses.push_back(0);
	}
	
	while (run && a.continueProcessing())
	{
		/*if (menuOpen) {
			a.stepMenu(GetTickCount() - stepTime);
		}
		else {
			a.step(GetTickCount() - stepTime);
		}*/

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

		if (draw)
		{
			cdepth.DrawClassification(pose);
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

		//if (menuOpen == false) {
		/*if (pose == 1) {
			cube1.setColour(1, 0, 0);
		}
		else if (pose == 2) {
			cube1.setColour(0, 1, 0);
		}
		else if (pose == 3) {
			cube1.setColour(0, 0, 1);
		}
		else {
			cube1.setColour(0, 0, 0);
		}

		cube1.doBuffers();*/
		int sensitivityScale = 10.0; //higher value, more need to move hand
		if (menuOpen == false && pose == 3 && poses[poses.size() - 5] == 3) {
			offsetx = -((float)cdepth.getWidth() / 2.0 - (float)hand1.xpos) / sensitivityScale;
			offsety = -((float)cdepth.getHeight() / 2.0 - (float)hand1.ypos) / sensitivityScale;
			offsetz = ((cdepth.getMaxDepth() - cdepth.getMinDepth()) / 2.0 - 80 - (float)hand1.depth + offsetz) / sensitivityScale;
		}
		cursor3d.setXYZ(((float)cdepth.getWidth() / 2.0 - (float)hand1.xpos) / sensitivityScale + offsetx, ((float)cdepth.getHeight() / 2.0 - (float)hand1.ypos + offsety) / sensitivityScale + offsety, -((cdepth.getMaxDepth() - cdepth.getMinDepth()) / 2.0 - 80 - (float)hand1.depth + offsetz) / sensitivityScale + offsetz);


		bool cubeChosen = false;

		for (int c = 0; c < cubes.size(); c++) {
			Cube* cube = cubes[c];
			float range = cube->getScaleFactor() + 0;

			//letting go of cube
			if (cubeState[c] == 1 && pose != 2 && poses.back() == 2 && abs(hand1.depth - prevHand1.depth) < 15 && abs(hand1.xpos - prevHand1.xpos) + abs(hand1.ypos - prevHand1.ypos) < 10) { // because can fly off a little if opening hand due to movement of central point and change of depth
				cube->getRigidBody()->setLinearVelocity(btVector3(0, 0, 0));
				cube->getRigidBody()->setAngularVelocity(btVector3(0, 0, 0));
				cubeState[c] = 0; //not doing anything
			}
			else if (pose != 2) { //throwing cube
				cubeState[c] = 0;
			}
			else if (cubeState[c] == 1) { //In state of being held
				a.applyForce(*cube, GetTickCount() - stepTime, (prevHand1.xpos - hand1.xpos) * 1000, (prevHand1.ypos - hand1.ypos) * 1000, (hand1.depth - prevHand1.depth) * 1000);
			}
			else if (pose == 2 && poses.back() != 2 && prevCursor3d.within(*cube, range, range, range) && cubeChosen == false) { //Grabbing the cube, use previous cursor position because cursor moves when grabbed
				cubeState[c] = 1; //1 = being grabbed
				cubeChosen = true;
				mostRecentCube = c;
			}
		
			if ((cubeChosen == false && cursor3d.within(*cube, range, range, range)) || cubeState[c] == 1) {
				cube->setColour(1, 1, 1);
				cube->doBuffers();
				cubeChosen = true;
				mostRecentCube = c;
			}
			else {
				cube->setColour(1, 1, 0);
				cube->doBuffers();
			}

			a.pushMat();
			a.updateCube(*cube);
			a.scale(cube->getScaleFactor(), cube->getScaleFactor(), cube->getScaleFactor()); //If double cube size, scale double as well, but after updated cube
			a.render(*cube);
			a.popMat();
		}

		if (cubeChosen == false && menuOpen == false) {
			a.pushMat();
			a.translate(cursor3d.xpos, cursor3d.ypos, cursor3d.zpos);
			a.scale(0.4, 0.4, 0.4);
			a.render(cursor3d);
			a.popMat();
		}

		//cube1.setColour(0, 0, 0);
		//cube1.doBuffers();
		

		

		/*a.pushMat();
		a.updateCube(stillCube);
		a.render(stillCube);
		a.popMat();*/
		//}
		/*if (menuOpen == false && grabbing == false && (cursor3d.xpos > 10 || cursor3d.xpos < -10) || cursor3d.ypos > 10 || cursor3d.ypos < 0 || cursor3d.zpos > 10 || cursor3d.zpos < -10) //add other dircetions
		{
			glDisable(GL_DEPTH_TEST);
			a.pushMat();
			cursor3d.setXYZ(((float)cdepth.getWidth() / 2.0 - (float)hand1.xpos) / 5.0, ((float)cdepth.getHeight() / 2.0 - (float)hand1.ypos) / 5.0, -((cdepth.getMaxDepth() - cdepth.getMinDepth()) / 2.0 - 80 - (float)hand1.depth) / 5.0);
			a.translate(cursor3d.xpos, cursor3d.ypos, cursor3d.zpos);
			a.scale(0.2, 0.2, 0.2);
			a.render(cursor3d);
			a.popMat();
			glEnable(GL_DEPTH_TEST);
		}
		else */
		

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
			//glDisable(GL_LIGHTING);
			

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
				if (pose == 3 && cursor.within(slider1, 15, 10) && cursor.within(tbar1,50,100)) { //move slider with finger. 100 is big enough to make sure whole slider covered, but is really irrelevant as this is just checking if slider is within x range of trackbar.
					a.setPosition(slider1, cursor.getRigidBody()->getCenterOfMassPosition()[0], slider1.getRigidBody()->getCenterOfMassPosition()[1], 0);
					slider1.setValue((abs(slider1.getRigidBody()->getCenterOfMassPosition()[0])-20)/2);
					float sizex = cubes[mostRecentCube]->getRigidBody()->getCenterOfMassPosition()[0];
					float sizey = cubes[mostRecentCube]->getRigidBody()->getCenterOfMassPosition()[1];
					float sizez = cubes[mostRecentCube]->getRigidBody()->getCenterOfMassPosition()[2];
					float newDist = 1.0f*slider1.getValue() / 10.0;
					if (sizex + newDist > 15) {
						a.setPosition(*cubes[mostRecentCube], 15 - newDist-0.1, sizey, sizez); //MAKE THIS HAVE DYMNAMIC SIZE i.e. not 15 but size of room 
					}
					if (sizex - newDist < -15) {
						a.setPosition(*cubes[mostRecentCube], -15 + newDist+0.1, sizey, sizez);
					}
					if (sizey - newDist < 0) {
						a.setPosition(*cubes[mostRecentCube], sizex, 1 + newDist, sizez);
					}
					if (sizez + newDist > 15) {
						a.setPosition(*cubes[mostRecentCube], sizex, sizey, 15 - newDist-0.1);
					}
					if (sizez - newDist < -15) {
						a.setPosition(*cubes[mostRecentCube], sizex, sizey, -15 + newDist+0.1);
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
				if (pose == 3 && cursor.within(slider2, 15, 10) && cursor.within(tbar2, 50, 100)) { //move slider with finger. 100 is big enough to make sure whole slider covered, but is really irrelevant as this is just checking if slider is within x range of trackbar.
					a.setPosition(slider2, cursor.getRigidBody()->getCenterOfMassPosition()[0], slider2.getRigidBody()->getCenterOfMassPosition()[1], 0);
					slider2.setValue((abs(slider2.getRigidBody()->getCenterOfMassPosition()[0]) - 20) / 2);

					cubes[mostRecentCube]->getRigidBody()->setMassProps(1.0f*slider2.getValue() / 10.0, cubes[mostRecentCube]->getRigidBody()->getLocalInertia());
					//a.setPosition(cube1, 0, 20, 0);
				}
				a.updatePosition(slider2);
				a.render(slider2);
			a.popMat();

			if (pose == 3) {
				cursor.setColour(1, 0, 0);
				cursor.doBuffers();
			}
			else {
				cursor.setColour(0.5, 0.5, 0.5);
				cursor.doBuffers();
			}
			a.pushMat();
				a.updatePosition(cursor);
				//a.scale(0.05, 0.05, 0);
				a.scale(0.2, 0.2, 0.2);
				a.render(cursor);
			a.popMat();

			//finger cursor
			/*if (pose == 3) {
				a.pushMat();
					a.translate(globalFingerX, globalFingerY, 0); //scale so finger tip appears closer
					a.scale(0.1, 0.1, 0);
					a.render(fingertipCursor);
				a.popMat();
			}*/
			glEnable(GL_LIGHTING);
			glEnable(GL_DEPTH_TEST);
			a.setP(p);

			menuOpen = true;
			menuCounter++;
		}
		else {
			menuClosedCounter++;
			glReadPixels(0, 0, justMenu.cols, justMenu.rows, GL_BGR, GL_UNSIGNED_BYTE, justMenu.data);
			a.lightingOn();
		}
		
		if(a.menuOpenGesture(poses) && menuCounter > frameCheck){ //go from open to closed
			menuOpen = false;
			menuCounter = 0;
		}

		if (a.menuOpenGesture(poses) && menuOpen == false && menuClosedCounter > frameCheck) { //open from closed
			menuOpen = true;
			menuClosedCounter = 0;
		}
		
		Mat menuImg(a.getHeight(), a.getWidth(), CV_8UC3); //just the sliders and cursor
		glReadPixels(0, 0, menuImg.cols, menuImg.rows, GL_BGR, GL_UNSIGNED_BYTE, menuImg.data);

		Mat translucentbackground(a.getHeight(), a.getWidth(), CV_8UC3);
		//Mat slidersAndCursor(a.getHeight(), a.getWidth(), CV_8UC3);
		//Mat combinedImg(a.getHeight(), a.getWidth(), CV_8UC3);

		addWeighted(noMenuImg, 0.5, justMenu, 0.5, 0, translucentbackground);
		//addWeighted(translucentbackground, 1, menuImg, 0.5, 0, translucentbackground);
		if (menuOpen) {
			addWeighted(translucentbackground, 1, menuImg, -1, 0, translucentbackground);
		}
		
		//cout << -1*(cursor.getRigidBody()->getCenterOfMassPosition()[0])+cdepth.getWidth()/2 << " " << hand1.xpos << endl;
		/*for (int r = 0; r < menuImg.rows; r++) {
			for (int c = 0; c < menuImg.cols; c++) {
				if (menuImg.at<Vec3b>(r, c) == justMenu.at<Vec3b>(r, c)) {
					slidersAndCursor.at<Vec3b>(r, c)[0] = 0;
					slidersAndCursor.at<Vec3b>(r, c)[1] = 0;
					slidersAndCursor.at<Vec3b>(r, c)[2] = 0;
				}
			}
		}*/
		a.setOglimg(translucentbackground);

		a.postprocessing();

		globalCounter++;
		prevHand1 = hand1;
		prevCursor3d.setXYZ(cursor3d.xpos, cursor3d.ypos, cursor3d.zpos);
		poses.pop_front();
		poses.push_back(pose);

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

