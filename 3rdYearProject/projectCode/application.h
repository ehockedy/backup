﻿#pragma once
#include "stdafx.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#define GLEW_STATIC
#include <glew.h>
#include <glfw3.h>   //glfw-3.2.1.bin.WIN64/include/GLFW/glfw3.h> //Lib for 3D maths
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

using namespace std;
using namespace glm;


GLint width = 1024;
GLint height = 768;
GLFWwindow* window;

//Matrices
mat4 Projection;
mat4 View;
mat4 Model;
mat4 MVP;

//mat4 Projection;
//mat4 View;
mat4 cubeModel;
mat4 cubeMVP;

stack<mat4> modelViewStack;
//stack<mat4> projectionStack;

void pushMat(mat4 m)
{
	mat4 temp;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			temp[i][j] = m[i][j];
		}
	}
	modelViewStack.push(temp);
}

mat4 popMat()
{
	mat4 temp;// = modelViewStack.top();
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			temp[i][j] = modelViewStack.top()[i][j];
		}
	}
	modelViewStack.pop();
	return temp;
}

void renderAttrib(int attrib, GLuint buffer)
{
	glEnableVertexAttribArray(attrib); //specify you want OpenGL to use vertex attribute arrays; without this call the data you supplied will be ignored.
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	//Specify the location and data format of the array of generic vertex attributes
	//tells OpenGL what to do with the supplied array data, since OpenGL doesn't inherently know what format that data will be in
	glVertexAttribPointer(
		attrib,                  // Specifies the index of the generic vertex attribute to be modified
		3,                  // Number of attributes (coordinates) per vertex
		GL_FLOAT,           // Data type of each component in the array
		GL_FALSE,           // Whether fixed-point data values should be normalized (GL_TRUE) or converted directly as fixed-point values (GL_FALSE) when they are accessed
		0,                  // Byte offset between consecutive generic vertex attributes. If stride​ is 0, the generic vertex attributes are understood to be tightly packed in the array
		(void*)0            // Specifies a offset of the first component of the first generic vertex attribute in the array in the data store of the buffer currently bound to the GL_ARRAY_BUFFER target.
	);
}

// Initial position : on +Z
vec3 position = vec3(0, 2, -15);
// Initial horizontal angle : toward -Z
float horizontalAngle = -3.14f / 2.0f;//-3.14f/2.0f;// 3.14f / 2.0f;
									  // Initial vertical angle : none
float verticalAngle = 0.0f;
// Initial Field of View
float initialFoV = 45.0f;

float speed = 10.0f; // 3 units / second
float mouseSpeed = 0.005f;
int r = 10; //Distance from the cube

void computeMatricesFromInputs() {

	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	// Get mouse position
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	// Reset mouse position for next frame
	glfwSetCursorPos(window, 1024 / 2, 768 / 2);

	// Compute new orientation
	horizontalAngle += mouseSpeed * float(1024 / 2 - xpos);
	verticalAngle += mouseSpeed * float(768 / 2 - ypos);

	// Direction : Spherical coordinates to Cartesian coordinates conversion
	vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);

	// Right vector
	vec3 right = vec3(
		sin(horizontalAngle - 3.14f / 2.0f),
		0,
		cos(horizontalAngle - 3.14f / 2.0f)
	);

	// Up vector
	vec3 up = cross(right, direction);

	// Move forward
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		position += direction * deltaTime * speed;
	}
	// Move backward
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		position -= direction * deltaTime * speed;
	}
	// Strafe right
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		position += right * deltaTime * speed;
	}
	// Strafe left
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		position -= right * deltaTime * speed;
	}

	float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

						   // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	Projection = perspective(FoV, 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	View = glm::lookAt(
		position,           // Camera is here
		position + direction, // and looks here : at the same position, plus "direction"
		up                  // Head is up (set to 0,-1,0 to look upside-down)
	);
	/*View = lookAt(
	position,           // Camera is here
	vec3(0,0,0), // and looks here : at the same position, plus "direction"
	vec3(0,1,0)                  // Head is up (set to 0,-1,0 to look upside-down)
	);*/

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}

class Object {
public:
	void doBuffers();
	GLuint* getVertexBuffer();
	GLuint* getColourBuffer();
	std::vector<GLfloat> getVertexData();
	std::vector<GLfloat> getColourData();

protected:
	Object();
	int numVertices;
	std::vector<GLfloat> vertices;
	std::vector<GLfloat> colours;
	GLuint vertexBuffer;
	GLuint colourBuffer;

};

class Cube : public Object {
public:
	Cube(GLfloat r, GLfloat g, GLfloat b);
	void setVertices();
	void setColour(GLfloat r, GLfloat g, GLfloat b);
};



class App {
public:
	void setup();
	GLuint getProgramID();
	GLuint getMatrixID();
	void preprocessing();
	void postprocessing();
private:
	GLuint programID;
	GLuint MatrixID;
};



Object::Object() {
}

Cube::Cube(GLfloat r, GLfloat g, GLfloat b) {
	numVertices = 9 * 12;
	setVertices();
	setColour(r, g, b);
	doBuffers();
}

vector<GLfloat> Object::getVertexData() {
	return vertices;
}

vector<GLfloat> Object::getColourData() {
	return colours;
}

GLuint* Object::getVertexBuffer() {
	return &vertexBuffer;
}

GLuint* Object::getColourBuffer() {
	return &colourBuffer;
}

void Object::doBuffers() {
	//GLuint vertexBuffer;
	//Creates 1 buffer, put the resulting identifier in vertexbuffer
	//This only creates the objects name, the reference to the object.
	glGenBuffers(1, &vertexBuffer);
	// This sets up its internal state
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	//Create and initialise the buffer objects data store
	//Do not put in any data, this just creates a fixed size buffer
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

	//GLuint colourBuffer;
	glGenBuffers(1, &colourBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colourBuffer);
	glBufferData(GL_ARRAY_BUFFER, colours.size() * sizeof(GLfloat), &colours[0], GL_STATIC_DRAW);
}

void Cube::setVertices() {
	vertices.clear();
	GLfloat data[9 * 12] = {
		-1.0f, -1.0f, -1.0f, // triangle 1 
		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,

		1.0f, 1.0f, -1.0f, // triangle 2
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,

		1.0f, -1.0f, 1.0f, //triangle 3
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		1.0f, 1.0f, -1.0f, //triangle 4
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f, //triangle 5
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,

		1.0f, -1.0f, 1.0f, //triangle 6
		-1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,

		-1.0f, 1.0f, 1.0f, //triangle 7
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,

		1.0f, 1.0f, 1.0f, //triangle 8
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,

		1.0f, -1.0f, -1.0f, //triangle 9
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,

		1.0f, 1.0f, 1.0f, //triangle 10
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,

		1.0f, 1.0f, 1.0f, //triangle 11
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,

		1.0f, 1.0f, 1.0f, //triangle 12
		-1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f
	};
	for (int i = 0; i < numVertices; i++)
	{
		vertices.push_back(data[i]);
	}
}

void Cube::setColour(GLfloat r, GLfloat g, GLfloat b) {
	colours.clear();
	for (int i = 0; i < numVertices / 3; i++)
	{
		colours.push_back(r);
		colours.push_back(g);
		colours.push_back(b);
	}
}

GLuint App::getProgramID() {
	return programID;
}

GLuint App::getMatrixID() {
	return MatrixID;
}

void App::preprocessing() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use our shader
	glUseProgram(programID);

	//Change the camera angle
	//computeMatricesFromInputs();
}

void App::postprocessing() {
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glfwSwapBuffers(window);
	glfwPollEvents();
}

void App::setup() {
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		//return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL 

																   // Open a window and create its OpenGL context
	window = glfwCreateWindow(width, height, "Tutorial 01", NULL, NULL); //Create the window
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		glfwTerminate();
		//return -1;
	}
	glfwMakeContextCurrent(window); // Initialize GLEW
	glewExperimental = true; // Needed in core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		//return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	glClearColor(0.0f, 0.0f, 0.1f, 0.0f);//Set the background colour

										 // Enable depth test - means closest things are draw over further away things
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	GLuint VertexArrayID; //A predefined OpenGL type to store an unsigned binary integer. This is different to an normal unsigned int in that it is a fixed size of 32 bis, as opposed to the size of a regular int whih can vary dependig on platform. This allows GL to work with the sizes it expects to.
	glGenVertexArrays(1, &VertexArrayID); //Specifies that 1 vertex array object name will be generated. 2nd arg is the array in which the generated vertex array object names are stored
	glBindVertexArray(VertexArrayID); //binds the vertex array object with name VertexArrayID

									  /////////
	programID = LoadShaders("vertexshader.txt", "fragmentshader.txt");

	//---------MATRICES---------

	// Initialize them by
	modelViewStack.push(mat4(1.0f));//Have to be done in a function

									// Get a handle for our "MVP" uniform
									/////////
	MatrixID = glGetUniformLocation(programID, "MVP");
	//GLuint cubeMatrixID = glGetUniformLocation(programID, "cubeMVP");

	//PROJECTION matrix
	//Applies perspective to the scene i.e. objects further away appear smaller than objects that are closer
	//FoV = 45.0°
	//Aspect ration = width:height
	//Display range = 0.1 to 100 units
	Projection = perspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);

	//VIEW matrix (camera matrix)
	//Represents the movement of the virtual camera around the scene
	View = lookAt( //function to create a viewing matrix
		vec3(0, 0, -20), //Camera position in world coordinates
		vec3(0, 0, 0), //Where the camera looks. Having the x and y coor from the cam pos and then negative (or any smaller) z value makes it look straight down the z axis if no transformations are being applied.
		vec3(0, 1, 0) //Head is up ((0,-1,0) looks upside down)
	);

	/*MODEL matrix
	The model matrix. Matrices can represent translations, rotations, and other geometrical transformations. You can use a single 4×4 matrix to represent any number of
	transformations in 3D space; you start with the identity matrix — that is, the matrix that represents a transformation that does nothing at all — then
	multiply it by the matrix that represents your first transformation, then by the one that represents your second transformation, and so on. The
	combined matrix represents all of your transformations in one. The matrix we use to represent this current move/rotate state is called the model matrix*/
	Model = mat4({ 1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f }); //An identity matrix

								   //MODEL VIEW PROJECTION matrix
	MVP = Projection * View * Model; //Remember matrix multiplication applies transformations from right to left
									 //cubeMVP = Projection * View * cubeModel;
									 //---------END MATRICES----------

}