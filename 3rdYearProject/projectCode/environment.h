#pragma once
#include "stdafx.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#define GLEW_STATIC
#include <glew.h>
#include <glfw3.h>   
#include <glm.hpp>
//#include <GL/glu.h>
#include <freeglut.h>
#include <gtx/transform.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/rotate_vector.hpp>
//#include <gtx/rotate_normalized_axis.hpp>
#include "shader.hpp"
#include <btBulletDynamicsCommon.h>
#include <stack>
#include <queue>
#include <vector>
#include <opencv2/opencv.hpp>

class Object {
public:
	void doBuffers();
	GLuint* getVertexBuffer(){ return &vertexBuffer; }
	GLuint* getColourBuffer(){ return &colourBuffer; }
	GLuint* getNormalBuffer() { return &normalBuffer; }
	std::vector<GLfloat> getVertexData() { return vertices; }
	std::vector<GLfloat> getColourData() { return colours; }
	int getNumVertices() { return numVertices; }
	void setColour(GLfloat r, GLfloat g, GLfloat b);
	btRigidBody* getRigidBody() { return rigidBody; };
	void setRigidBody(btRigidBody* rb) { rigidBody = rb; };
	btScalar getMass() { return mass; };
	void setMass(btScalar m) { mass = m; };
	std::string getID() { return id; };
	void setVertices(std::vector<GLfloat> data);
	void setNormals(std::vector<GLfloat> data);
	bool within(Object o, float xrange, float yrange);
	bool within(Object o, float xrange, float yrange, float zrange);
	void setScaleFactor(float sf) { scaleFactor = sf; };
	float getScaleFactor() { return scaleFactor; };
	void setXYZ(float x, float y, float z) { xpos = x; ypos = y; zpos = z; };
	float xpos;
	float ypos;
	float zpos;
	void setNum(int n) { num = n; };
	int getNum() { return num; };
protected:
	Object();
	int numVertexAttributes;
	int numVertices;
	std::vector<GLfloat> vertices;
	std::vector<GLfloat> colours;
	std::vector<GLfloat> normals;
	GLuint vertexBuffer;
	GLuint colourBuffer;
	GLuint normalBuffer;

	btCollisionShape* collisionShape; 
	btDefaultMotionState* motionState;
	btRigidBody* rigidBody;
	btScalar mass;

	std::string id;
	float scaleFactor;
	int num;
};

class Cube : public Object {
public:
	Cube();
	//void setVertices();
	btRigidBody* setUpPhysics(btVector3 pos, btScalar m, btVector3 dim);
};

class Plane : public Object {
public:
	Plane();
	//void setVertices();
	btRigidBody* setUpPhysics(btVector3 norm, btVector3 pos);
	btStaticPlaneShape* getPlaneShape() { return planeShape; };
private:
	btStaticPlaneShape* planeShape;
};

class Menu2D : public Object {
public:
	Menu2D();
	//void setVertices();
	//btRigidBody* setUpPhysics(btVector3 norm, btVector3 pos);
};

class Trackbar : public Object {
public:
	Trackbar();
	btRigidBody* setUpPhysics(btVector3 pos);
	//void setVertices();
};

class Cursor : public Object {
public:
	Cursor();
	btRigidBody* setUpPhysics(btVector3 pos);
	//void setVertices();
	bool setXYZ(float x, float y, float z) { xpos = x; ypos = y; zpos = z; };
private:
	float xpos;
	float ypos;
	float zpos;
};

class Slider : public Object {
public:
	Slider();
	void setValue(float val) { value = val; };
	float getValue() { return value; };
	float getXPos() { return xpos; };
	float getYPos() { return ypos; };
	void setX(float x) { xpos = x; };
	void setMinX(float x) { minX = x; };
	void setMaxX(float x) { maxX = x; };
	float getMinX() { return minX; };
	float getMaxX() { return maxX; };
	void setY(float y) { ypos = y; };
	bool isWithin(float x, float y);
	btRigidBody* setUpPhysics(btVector3 pos);
private:
	float xpos;
	float ypos;
	float zpos;
	float value = 1.0;
	float minValue = 0.1;
	float maxValue = 5.0;
	float minX;
	float maxX;
};

class App {
public:
	App();
	void setupView();
	bool continueProcessing();
	void preprocessing(bool move);
	void postprocessing();
	void renderAttrib(int attrib, GLuint buffer, int numAtt);
	void pushMat();
	void popMat();
	void translate(float x, float y, float z);
	void rotate(float angle, float x, float y, float z);
	void scale(float x, float y, float z);
	void render(Object o);
	void setCamera(float x, float y, float z) { position = glm::vec3(x, y, z); };
	void setupPhysics();
	void addToWorld(btRigidBody* rigidBody);
	void addToMenuWorld(btRigidBody* rigidBody);
	void updateCube(Cube c);
	void updatePlane(Plane p);
	void updatePosition(Object o);
	void setPosition(Object o, float x, float y, float z);
	void step(unsigned long time);
	void stepMenu(unsigned long time);
	void applyForce(Object o, unsigned long time, float sx, float sy, float sz);
	void applyNegativeGravity(Object o);
	int getWidth() { return width; };
	int getHeight() { return height; };
	void outputImage(std::string name, cv::Mat handsImg);
	void outputImage(std::string name);
	void setP(glm::mat4 mat) { Projection = mat; };
	glm::mat4 getP() { return Projection; };
	bool menuOpenGesture(std::deque<int> poses);
	bool menuCloseGesture(std::deque<int> poses);
	bool resetGesture(std::deque<int> poses);
	btVector3 getGravity() { return dynamicsWorldMenu->getGravity(); };
	void setOglimg(cv::Mat img) { oglimg = img; };
	void menuLightingOff() {glUniform1i(menuUniformLocation, 1);};
	void lightingOn() { glUniform1i(menuUniformLocation, 0); };
	void makeCube(float x, float y, float z, float mass, float xdim, float ydim, float zdim, std::vector<Cube*> *cubes, std::vector<int> *cubeState, std::vector<int> *cubeState2);
	Cube* replace(Object o, float x, float y, float z, float mass, float xdim, float ydim, float zdim);
private:
	btBroadphaseInterface* broadphase = new btDbvtBroadphase(); //Decide on the Broadphase algorithm - uses the bounding boxes of objects in the world to quickly compute a conservative approximate list of colliding pairs
	btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration(); //allows configuration of Bullet collision stack allocator and pool memory allocators
	btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver; //Solver - causes the objects to intersct properly, taking into account gravity, game logic supplied force, collisions, and hinge constraints
	btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration); //Instantiate dynamics world
	
	btBroadphaseInterface* broadphaseMenu = new btDbvtBroadphase(); //Decide on the Broadphase algorithm - uses the bounding boxes of objects in the world to quickly compute a conservative approximate list of colliding pairs
	btDefaultCollisionConfiguration* collisionConfigurationMenu = new btDefaultCollisionConfiguration(); //allows configuration of Bullet collision stack allocator and pool memory allocators
	btCollisionDispatcher* dispatcherMenu = new btCollisionDispatcher(collisionConfigurationMenu);
	btSequentialImpulseConstraintSolver* solverMenu = new btSequentialImpulseConstraintSolver; //Solver - causes the objects to intersct properly, taking into account gravity, game logic supplied force, collisions, and hinge constraints
	btDiscreteDynamicsWorld* dynamicsWorldMenu = new btDiscreteDynamicsWorld(dispatcherMenu, broadphaseMenu, solverMenu, collisionConfigurationMenu); //Instantiate dynamics world


	GLuint programID;
	GLuint MatrixID;

	GLint width = 1300;// 1024;
	GLint height = 700;// 768;
	GLFWwindow* window;

	//Matrices
	glm::mat4 Projection;
	glm::mat4 View;
	glm::mat4 Model;
	glm::mat4 MVP;

	glm::mat4 cubeModel;
	glm::mat4 cubeMVP;

	GLuint LightID;
	GLuint ViewMatrixID;
	GLuint ModelMatrixID;
	GLuint menuUniformLocation;

	std::stack<glm::mat4> modelViewStack;

	// Initial position : on +Z
	glm::vec3 position = glm::vec3(0, 0, -15);
	// Initial horizontal angle : toward -Z
	float horizontalAngle = -3.14f / 2.0f;

	// Initial vertical angle : none
	float verticalAngle = 0.0f;
	// Initial Field of View
	float initialFoV = 45.0f;

	float speed = 10.0f; // 3 units / second
	float mouseSpeed = 0.005f;
	int r = 10; //Distance from the cube

	GLuint getProgramID() { return programID; }
	GLuint getMatrixID() { return MatrixID; }
	GLFWwindow* getWindow() { return window; }

	void computeMatricesFromInputs();
	glm::mat4 getModel() { return Model; }

	cv::Mat oglimg;
};
