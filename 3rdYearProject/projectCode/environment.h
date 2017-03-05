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
#include <vector>

class Object {
public:
	void doBuffers();
	GLuint* getVertexBuffer(){ return &vertexBuffer; }
	GLuint* getColourBuffer(){ return &colourBuffer; }
	std::vector<GLfloat> getVertexData() { return vertices; }
	std::vector<GLfloat> getColourData() { return colours; }
	int getNumVertices() { return numVertices; }
	void setColour(GLfloat r, GLfloat g, GLfloat b);
	btRigidBody* getRigidBody() { return rigidBody; };
	void setRigidBody(btRigidBody* rb) { rigidBody = rb; };
	btScalar getMass() { return mass; };
protected:
	Object();
	int numVertexAttributes;
	int numVertices;
	std::vector<GLfloat> vertices;
	std::vector<GLfloat> colours;
	GLuint vertexBuffer;
	GLuint colourBuffer;

	btCollisionShape* collisionShape; 
	btDefaultMotionState* motionState;
	btRigidBody* rigidBody;
	btScalar mass;
};

class Cube : public Object {
public:
	Cube();
	void setVertices();
	btRigidBody* setUpPhysics();
};

class Plane : public Object {
public:
	Plane();
	void setVertices();
	btRigidBody* setUpPhysics(btVector3 norm, btVector3 pos);
	btStaticPlaneShape* getPlaneShape() { return planeShape; };
private:
	btStaticPlaneShape* planeShape;
};



class App {
public:
	void setupView();
	bool continueProcessing();
	void preprocessing(bool move);
	void postprocessing();
	void renderAttrib(int attrib, GLuint buffer);
	void pushMat();
	void popMat();
	void translate(float x, float y, float z);
	void rotate(float angle, float x, float y, float z);
	void scale(float x, float y, float z);
	void render(Object o);
	void setCamera(float x, float y, float z) { position = glm::vec3(x, y, z); };
	void setupPhysics();
	void addToWorld(btRigidBody* rigidBody);
	void updateCube(Cube c);
	void updatePlane(Plane p);
	void updatePosition(Object o);
	void setPosition(Object o, float x, float y, float z);
	void step(unsigned long time);
	void applyForce(Object o, unsigned long time, float sx, float sy, float sz);
private:
	btBroadphaseInterface* broadphase = new btDbvtBroadphase(); //Decide on the Broadphase algorithm - uses the bounding boxes of objects in the world to quickly compute a conservative approximate list of colliding pairs
	btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration(); //allows configuration of Bullet collision stack allocator and pool memory allocators
	btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);

	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver; //Solver - causes the objects to intersct properly, taking into account gravity, game logic supplied force, collisions, and hinge constraints

	btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration); //Instantiate dynamics world
	

	GLuint programID;
	GLuint MatrixID;

	GLint width = 1024;
	GLint height = 768;
	GLFWwindow* window;

	//Matrices
	glm::mat4 Projection;
	glm::mat4 View;
	glm::mat4 Model;
	glm::mat4 MVP;

	glm::mat4 cubeModel;
	glm::mat4 cubeMVP;

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
};