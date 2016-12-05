//#include "stdafx.h"
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

using namespace std;
using namespace glm;

//Window
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

// Initial position : on +Z
vec3 position = vec3(0, 2, -15);
// Initial horizontal angle : toward -Z
float horizontalAngle = -3.14f/2.0f;//-3.14f/2.0f;// 3.14f / 2.0f;
// Initial vertical angle : none
float verticalAngle = 0.0f;
// Initial Field of View
float initialFoV = 45.0f;

float speed = 10.0f; // 3 units / second
float mouseSpeed = 0.005f;
int r = 10; //Distance from the cube

/*void computeMatricesFromInputs() { //CHANGE THIS SO THAT ONLY USE 6 KEYS FOR MOVING IN 6 DIRECTIONS

	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	//vec3 position(r * sin(horizontalAngle - 3.14f), r * verticalAngle, r * cos(horizontalAngle - 3.14f)); //-3.14 makes it go between -pi and +pi
	
	// Up vector
	vec3 up = vec3(0, 1, 0);//cross(turnxz, direction); 

	// Move forward
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		verticalAngle += mouseSpeed;
		vec3 turnyz = vec3(0, verticalAngle, 0);// 1 * cos(verticalAngle - 3.14f));
		position += turnyz * deltaTime * speed;
	}
	// Move backward
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		verticalAngle -= mouseSpeed;
		//vec3 turnyz = vec3(0, 1 * sin(verticalAngle - 3.14f), 1 * cos(verticalAngle - 3.14f));
		vec3 turnyz = vec3(0, verticalAngle, 0);
		position -= turnyz * deltaTime * speed;
	}
	// Strafe right
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		horizontalAngle += mouseSpeed;
		vec3 turnxz = vec3(1 * sin(horizontalAngle - 3.14f), 0, 1 * cos(horizontalAngle - 3.14f ));
		position += turnxz * deltaTime * speed;
	}
	// Strafe left
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		horizontalAngle -= mouseSpeed;
		vec3 turnxz = vec3(1 * sin(horizontalAngle - 3.14f), 0, 1 * cos(horizontalAngle - 3.14f));
		position -= turnxz * deltaTime * speed;
	}


	float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	Projection = perspective(FoV, 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	View = lookAt(
		position,           // Camera is here
		//position + direction, // and looks here : at the same position, plus "direction"
		vec3(0,0,0),
		up                  // Head is up (set to 0,-1,0 to look upside-down)
		);

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}*/




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
	View = lookAt(
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


void renderAttrib(int attrib, GLuint vertexBuffer)
{
	glEnableVertexAttribArray(attrib); //specify you want OpenGL to use vertex attribute arrays; without this call the data you supplied will be ignored.
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
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

GLuint generateColours(GLuint *buff, vec3 col, GLfloat *buffData)
{
	for (float i = 0.0; i < sizeof(buffData)/3; i++)
	{
		buffData[(int)(3.0 * i + 0)] = 1.0f;
		buffData[(int)(3.0 * i + 1)] = 0.0f;
		buffData[(int)(3.0 * i + 2)] = 1.0f;
	}

	glGenBuffers(1, buff);
	glBindBuffer(GL_ARRAY_BUFFER, *buff);
	glBufferData(GL_ARRAY_BUFFER, sizeof(buffData), buffData, GL_STATIC_DRAW);
	return *buff;

}


int main(int argc, char** argv)
{
	//Initilize GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}
	/*glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);

	//glutInit(&argc, argv);
	//glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(550, 550);
	glutInitWindowPosition(50, 50);
	glutCreateWindow("Balloon");*/

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
		return -1;
	}
	glfwMakeContextCurrent(window); // Initialize GLEW
	glewExperimental = true; // Needed in core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
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

	GLuint programID = LoadShaders("vertexshader.txt", "fragmentshader.txt");

	//---------MATRICES---------

	// Initialize them by
	modelViewStack.push(mat4(1.0f));//Have to be done in a function
	//projectionStack.push(mat4(1.0f));

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
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
		vec3(0, 1, 0), //Camera position in world coordinates
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


	//----------VERTICES---------
	//Vertices of a cube. Three consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
	//A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3 vertices
	//x direction is left to right
	//y direction is down to up
	//z direction is behind to in front
	//In each direction, coordinates go from -1 to 1 i.e. this triangle will take up most of the screen
	//static const 
	int numVertices = 12 * 3 * 3;

	static GLfloat cubeVertexBufferData[] = {
			-1.0f,-1.0f,-1.0f, // triangle 1 
			-1.0f,-1.0f, 1.0f,
			-1.0f, 1.0f, 1.0f, 

			1.0f, 1.0f,-1.0f, // triangle 2
			-1.0f,-1.0f,-1.0f,
			-1.0f, 1.0f,-1.0f,
		
			1.0f,-1.0f, 1.0f, //triangle 3
			-1.0f,-1.0f,-1.0f,
			1.0f,-1.0f,-1.0f,

			1.0f, 1.0f,-1.0f, //triangle 4
			1.0f,-1.0f,-1.0f,
			-1.0f,-1.0f,-1.0f,

			-1.0f,-1.0f,-1.0f, //triangle 5
			-1.0f, 1.0f, 1.0f,
			-1.0f, 1.0f,-1.0f,

			1.0f,-1.0f, 1.0f, //triangle 6
			-1.0f,-1.0f, 1.0f,
			-1.0f,-1.0f,-1.0f,

			-1.0f, 1.0f, 1.0f, //triangle 7
			-1.0f,-1.0f, 1.0f,
			1.0f,-1.0f, 1.0f,

			1.0f, 1.0f, 1.0f, //triangle 8
			1.0f,-1.0f,-1.0f,
			1.0f, 1.0f,-1.0f,

			1.0f,-1.0f,-1.0f, //triangle 9
			1.0f, 1.0f, 1.0f,
			1.0f,-1.0f, 1.0f,

			1.0f, 1.0f, 1.0f, //triangle 10
			1.0f, 1.0f,-1.0f,
			-1.0f, 1.0f,-1.0f,

			1.0f, 1.0f, 1.0f, //triangle 11
			-1.0f, 1.0f,-1.0f,
			-1.0f, 1.0f, 1.0f,

			1.0f, 1.0f, 1.0f, //triangle 12
			-1.0f, 1.0f, 1.0f,
			1.0f,-1.0f, 1.0f
		};

	/*static GLfloat cubeVertexBufferData[] = {
		0.0f, 0.0f, 0.0f, // triangle 1 
		0.0f, 0.0f, 2.0f,
		0.0f, 2.0f, 2.0f,

		2.0f, 2.0f,0.0f, // triangle 2
		0.0f,0.0f,0.0f,
		0.0f, 2.0f,0.0f,

		2.0f,0.0f, 2.0f, //triangle 3
		0.0f,0.0f,0.0f,
		2.0f,0.0f,0.0f,

		2.0f, 2.0f,0.0f, //triangle 4
		2.0f,0.0f,0.0f,
		0.0f,0.0f,0.0f,

		0.0f,0.0f,0.0f, //triangle 5
		0.0f, 2.0f, 2.0f,
		0.0f, 2.0f,0.0f,

		2.0f,0.0f, 2.0f, //triangle 6
		0.0f,0.0f, 2.0f,
		0.0f,0.0f,0.0f,

		0.0f, 2.0f, 2.0f, //triangle 7
		0.0f,0.0f, 2.0f,
		2.0f,0.0f, 2.0f,

		2.0f, 2.0f, 2.0f, //triangle 8
		2.0f,0.0f,0.0f,
		2.0f, 2.0f,0.0f,

		2.0f,0.0f,0.0f, //triangle 9
		2.0f, 2.0f, 2.0f,
		2.0f,0.0f, 2.0f,

		2.0f, 2.0f, 2.0f, //triangle 10
		2.0f, 2.0f,0.0f,
		0.0f, 2.0f,0.0f,

		2.0f, 2.0f, 2.0f, //triangle 11
		0.0f, 2.0f, 0.0f,
		0.0f, 2.0f, 2.0f,

		2.0f, 2.0f, 2.0f, //triangle 12
		0.0f, 2.0f, 2.0f,
		2.0f, 0.0f, 2.0f
	};*/

	//Buffer objects store an array of unformatted memory allocated by the OpenGL context (GPU)
	//They can be used to store vertex data, or other things
	// This will identify our vertex buffer
	GLuint cubeVertexBuffer;

	// Creates 1 buffer, put the resulting identifier in vertexbuffer
	//This only creates the objects name, the reference to the object.
	glGenBuffers(1, &cubeVertexBuffer);

	// This sets up its internal state
	glBindBuffer(GL_ARRAY_BUFFER, cubeVertexBuffer);

	//Create and initialise the buffer objects data store
	//Do not put in any data, this just creates a fixed size buffer
	//glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data) + sizeof(floorVertexBuffer), NULL, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertexBufferData), cubeVertexBufferData, GL_STATIC_DRAW);

	//FLOOR
	static GLfloat floorVertexBufferData[] = {
		-10.0f, 0.0f, -10.0f, //back left
		-10.0f, 0.0f, 10.0f, //front left
		10.0f, 0.0f, -10.0f, //back right

		10.0f, 0.0f, -10.0f, //back right
		10.0f, 0.0f, 10.0f, //front right
		-10.0f, 0.0f, 10.0f, //front left
	};

	GLuint floorVertexBuffer;

	// Creates 1 buffer, put the resulting identifier in vertexbuffer
	//This only creates the objects name, the reference to the object.
	glGenBuffers(1, &floorVertexBuffer);

	// This sets up its internal state
	glBindBuffer(GL_ARRAY_BUFFER, floorVertexBuffer);

	//Create and initialise the buffer objects data store
	//Do not put in any data, this just creates a fixed size buffer
	//glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data) + sizeof(floorVertexBuffer), NULL, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertexBufferData), floorVertexBufferData, GL_STATIC_DRAW);



	static GLfloat wallVertexBufferData[] = {
		(float)-(10.0 + 5.0*sqrt(2)), 0.0f, -10.0f, //back left
		-10.0f, 0.0f, 0.0f, //front left
		(float)(10.0 + 5.0*sqrt(2)), 0.0f, -10.0f, //back right

		(float)(10.0+5.0*sqrt(2)), 0.0f, -10.0f, //back right
		10.0f, 0.0f, 0.0f, //front right
		-10.0f, 0.0f, 0.0f, //front left
	};

	GLuint wallVertexBuffer;
	glGenBuffers(1, &wallVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, wallVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(wallVertexBufferData), wallVertexBufferData, GL_STATIC_DRAW);

	/*const int stacks = 2;
	const int slices = 2;
	int radius = 5;
	const int sphereSize = stacks*slices;
	static GLfloat sphereVertices[sphereSize*3];
	
	// Calc The Vertices
	for (int i = 0; i <= stacks; ++i) {

		float V = i / (float)stacks;
		float phi = V * glm::pi <float>();

		// Loop Through Slices
		for (int j = 0; j <= slices; ++j) {

			float U = j / (float)slices;
			float theta = U * (glm::pi <float>() * 2);

			// Calc The Vertex Positions
			float x = cosf(theta) * sinf(phi);
			float y = cosf(phi);
			float z = sinf(theta) * sinf(phi);
			cout << i*stacks*3 + (3 * j) << " " << i*stacks*3 + (3 * j) + 1 << " " << i*stacks*3 + (3 * j) + 2 << endl;
			// Push Back Vertex Data
			sphereVertices[i*slices*3 + (3*j)] = (x*radius);
			sphereVertices[i*slices*3 + (3*j)+1] = (y*radius);
			sphereVertices[i*slices*3 + (3*j)+2] = (z*radius);
		}
	}
	GLuint sphereVertexBuffer;
	glGenBuffers(1, &sphereVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphereVertexBuffer), sphereVertices, GL_STATIC_DRAW);*/

	//----------COLOURS----------
	//CUBE - GREEN
	static GLfloat cubeColourData[12*3*3];
	for (float i = 0.0; i < 12 * 3; i++)
	{
		cubeColourData[(int)(3.0 * i + 0)] = 0.0f;
		cubeColourData[(int)(3.0 * i + 1)] = 1.0f;
		cubeColourData[(int)(3.0 * i + 2)] = 0.0f;
	}
	GLuint cubeColourBuffer;
	glGenBuffers(1, &cubeColourBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, cubeColourBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeColourData), cubeColourData, GL_STATIC_DRAW);

	//LEFT WALL - MAGENTA
	static GLfloat leftColourData[2 * 3 * 3];
	for (float i = 0.0; i < 2*3 ; i++)
	{
		leftColourData[(int)(3.0 * i + 0)] = 1.0f;
		leftColourData[(int)(3.0 * i + 1)] = 0.0f;
		leftColourData[(int)(3.0 * i + 2)] = 1.0f;
	}
	GLuint leftColourBuffer;
	glGenBuffers(1, &leftColourBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, leftColourBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(leftColourData), leftColourData, GL_STATIC_DRAW);

	//FAR WALL - YELLOW
	static GLfloat farColourData[2*3*3];
	for (float i = 0; i < 2*3; i++)
	{
		farColourData[(int)(3.0 * i + 0)] = 1.0f;// fmod((i / 36), 1);
		farColourData[(int)(3.0 * i + 1)] = 1.0f;// fmod((i * 2) / 36, 1);
		farColourData[(int)(3.0 * i + 2)] = 0.0f;// fmod((i * 3) / 36, 1);
	}
	GLuint farColourBuffer;
	glGenBuffers(1, &farColourBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, farColourBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(farColourData), farColourData, GL_STATIC_DRAW);

	//NEAR WALL - CYAN
	static GLfloat nearColourData[2 * 3 * 3];
	for (float i = 0.0; i < 2 * 3; i++)
	{
		nearColourData[(int)(3.0 * i + 0)] = 0.0f;
		nearColourData[(int)(3.0 * i + 1)] = 1.0f;
		nearColourData[(int)(3.0 * i + 2)] = 1.0f;
	}
	GLuint nearColourBuffer;
	glGenBuffers(1, &nearColourBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, nearColourBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(nearColourData), nearColourData, GL_STATIC_DRAW);

	//RIGHT WALL - BLUE
	static GLfloat rightColourData[2 * 3 * 3];
	for (float i = 0.0; i < 2 * 3; i++)
	{
		rightColourData[(int)(3.0 * i + 0)] = 0.0f;
		rightColourData[(int)(3.0 * i + 1)] = 0.0f;
		rightColourData[(int)(3.0 * i + 2)] = 1.0f;
	}
	GLuint rightColourBuffer;
	glGenBuffers(1, &rightColourBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, rightColourBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rightColourData), rightColourData, GL_STATIC_DRAW);

	//FLOOR - RED
	static GLfloat floorColourData[2 * 3 * 3];
	for (float i = 0.0; i < 2 * 3; i++)
	{
		floorColourData[(int)(3.0 * i + 0)] = 1.0f;
		floorColourData[(int)(3.0 * i + 1)] = 0.0f;
		floorColourData[(int)(3.0 * i + 2)] = 0.0f;
	}
	GLuint floorColourBuffer;
	glGenBuffers(1, &floorColourBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, floorColourBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floorColourData), floorColourData, GL_STATIC_DRAW);

	
	/*static GLfloat g_color_buffer_data3[stacks*slices*3]; 
	for (float i = 0; i < stacks*slices; i++)
	{
		g_color_buffer_data3[(int)(3.0 * i + 0)] = 0.0f;// fmod((i / 36), 1);
		g_color_buffer_data3[(int)(3.0 * i + 1)] = 1.0f;// fmod((i * 2) / 36, 1);
		g_color_buffer_data3[(int)(3.0 * i + 2)] = 1.0f;// fmod((i * 3) / 36, 1);
	}
	
	GLuint colorbuffer3;
	glGenBuffers(1, &colorbuffer3);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer3);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data3), g_color_buffer_data3, GL_STATIC_DRAW);*/


	//BULLET__________________________________________
	//First lines of any Bullet application
	//------------------
	btBroadphaseInterface* broadphase = new btDbvtBroadphase(); //Decide on the Broadphase algorithm - uses the bounding boxes of objects in the world to quickly compute a conservative approximate list of colliding pairs
	btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration(); //allows configuration of Bullet collision stack allocator and pool memory allocators
	btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);

	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver; //Solver - causes the objects to intersct properly, taking into account gravity, game logic supplied force, collisions, and hinge constraints

	btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration); //Instantiate dynamics world
	dynamicsWorld->setGravity(btVector3(0, -9.81, 0)); //Set gravity
													 //-----------------


	//COLLISION SHAPES - A collision shape determines collisions, it has no concept of mass inertia, etc. Many bodies can share a collision shape, but they should be the same shape.
	//Create ground shape
	btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 1); //Create a static rigid body that will be the ground plane. the y value is 1 because of another plane with y value -1 that will be created later on that will have the overall effect of y = 0. Couls also have the 2 planes have y = 0
	btCollisionShape* leftWallShape = new btStaticPlaneShape(btVector3(1, 1, 0), 1);
	btCollisionShape* farWallShape = new btStaticPlaneShape(btVector3(0, 1, -1), 1);
	btCollisionShape* rightWallShape = new btStaticPlaneShape(btVector3(-1, 1, 0), 1);
	btCollisionShape* nearWallShape = new btStaticPlaneShape(btVector3(0, 1, 1), 1);
	//Create sphere
	btCollisionShape* fallShape = new btBoxShape(btVector3(1.0f,1.0f,1.0f)); //A box that will fall from the sky with sides of 2
	btCollisionShape* fallShape2 = new btBoxShape(btVector3(1.0f, 1.0f, 1.0f));

	//RIGID BODIES
	//Instantiate ground rigid body
	btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -1, 0))); //Instantiate the ground. Orientation is identity. Position is 1 meter below ground. This compensastes the 1m offset the shape itself had.
	btDefaultMotionState* leftWallState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(-10, 0, 0))); 
	btDefaultMotionState* farWallState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 10)));
	btDefaultMotionState* rightWallState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(10, 0, 0)));
	btDefaultMotionState* nearWallState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, -10))); 


	//Combine shape and rigid body
	btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0)); //btRigidBodyConstructionInfo structure provides info to create a rigid body. First parameter is mass. When mass is zero, Bullet treats it as immovable i.e. infinite mass. 2nd and 3rd params are the motion shape (coupe of lienes above) and collision shape (in the shape section). Final param is inertia.
	btRigidBody::btRigidBodyConstructionInfo leftRigidBodyCI(0, leftWallState, leftWallShape, btVector3(0, 0, 0));
	btRigidBody::btRigidBodyConstructionInfo farRigidBodyCI(0, farWallState, farWallShape, btVector3(0, 0, 0));
	btRigidBody::btRigidBodyConstructionInfo rightRigidBodyCI(0, rightWallState, rightWallShape, btVector3(0, 0, 0));
	btRigidBody::btRigidBodyConstructionInfo nearRigidBodyCI(0, nearWallState, nearWallShape, btVector3(0, 0, 0));
	//Create actual rigid body
	btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
	btRigidBody* leftRigidBody = new btRigidBody(leftRigidBodyCI);
	btRigidBody* farRigidBody = new btRigidBody(farRigidBodyCI);
	btRigidBody* rightRigidBody = new btRigidBody(rightRigidBodyCI);
	btRigidBody* nearRigidBody = new btRigidBody(nearRigidBodyCI);
	//Add groud to the world
	dynamicsWorld->addRigidBody(groundRigidBody);
	dynamicsWorld->addRigidBody(leftRigidBody);
	dynamicsWorld->addRigidBody(farRigidBody);
	dynamicsWorld->addRigidBody(rightRigidBody);
	dynamicsWorld->addRigidBody(nearRigidBody);

	//Intantiate sphere rigid body
	btDefaultMotionState* fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 10, 0)));
	btDefaultMotionState* fallMotionState2 = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(-2, 8, 0)));
	btScalar mass = 1; //Give it a mass of 1kg
	btVector3 fallInertia(0, 0, 0); //Initial inertia
	fallShape->calculateLocalInertia(mass, fallInertia); //Calculates the inertia as it falls
	btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState, fallShape, fallInertia); //If want to create lots of the bodies with the same parameters, only need to create one of these and pass that info to each body that is made
	btRigidBody::btRigidBodyConstructionInfo fallRigidBody2CI(mass, fallMotionState2, fallShape2, fallInertia); //If want to create lots of the bodies with the same parameters, only need to create one of these and pass that info to each body that is made
	btRigidBody* fallRigidBody = new btRigidBody(fallRigidBodyCI); //Create rigid body
	btRigidBody* fallRigidBody2 = new btRigidBody(fallRigidBody2CI);
	fallRigidBody->setLinearVelocity(btVector3(10, 0, 15));
	fallRigidBody2->setLinearVelocity(btVector3(12, 0, 12));
	//fallRigidBody->setRotation(1, 10, 10, 10);
	dynamicsWorld->addRigidBody(fallRigidBody); //Add to the world
	dynamicsWorld->addRigidBody(fallRigidBody2);
	//bool print = true;
	//int counter = 0;
	//btScalar prevY = 0;
	//STEP THE SIMULATION
	//for (int i = 0; i < 300; i++)
	float scaleFactor = 0.85;
	do
	{
		//BULLET STUFF
		dynamicsWorld->stepSimulation(1 / 1200.f, 10);

		btTransform trans; //Stores the state (position and orientation) of the falling sphere
		fallRigidBody->getMotionState()->getWorldTransform(trans);

		btScalar m[16];
		trans.getOpenGLMatrix(m);
		//cout << m[0] << " " << m[1] << " " << m[5] << endl;
		mat4 rotpos = mat4({m[0], m[1], m[2], m[3],
							m[4], m[5], m[6], m[7], 
							m[8], m[9], m[10], m[11], 
							m[12], m[13], m[14], m[15] });
		//cout << "rotation: " << trans.getRotation().x() << " " << trans.getRotation().y() << " " << trans.getRotation().z() << endl; // trans.getOrigin().getY() << " deltaY: " << deltaY << endl; //Only want the y position, got via getOrigin().getY()
		btTransform trans2; //Stores the state (position and orientation) of the falling sphere
		fallRigidBody2->getMotionState()->getWorldTransform(trans2);

		btScalar m2[16];
		trans2.getOpenGLMatrix(m2);
		//cout << m[0] << " " << m[1] << " " << m[5] << endl;
		mat4 rotpos2 = mat4({ m2[0], m2[1], m2[2], m2[3],
			m2[4], m2[5], m2[6], m2[7],
			m2[8], m2[9], m2[10], m2[11],
			m2[12], m2[13], m2[14], m2[15] });

		//---OPENGL---

		//Clear the screen to the colour specified earlier, as well as the depth
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		//Change the camera angle
		computeMatricesFromInputs();

		///glMatrixMode(GL_MODELVIEW);
		//glScalef(0.5, 0.5, 0.5);
		//Model = scale(Model, vec3(0.9f, 0.9f, 0.9f));
		
		//FLOOR
		pushMat(Model);
			Model = scale(Model, vec3((1.0*scaleFactor), (1.0*scaleFactor), (1.0*scaleFactor)));
			MVP = Projection * View * Model;
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);// Send our transformation to the currently bound shader, in the "MVP" uniform
			renderAttrib(0, floorVertexBuffer); //Floor
			renderAttrib(1, floorColourBuffer); //Colours
			glDrawArrays(GL_TRIANGLES, 0, 2 * 3); //2*3 is 2 triangles of 3 vertices
		Model = popMat();

		//LEFT WALL - MAGENTA
		pushMat(Model);
			Model = translate(Model, vec3(10 * scaleFactor, 0, 0));
			Model = rotate(Model, -3.14159f/2.0f, vec3(0,1,0));
			Model = rotate(Model, 3.14159f/4.0f, vec3(1, 0, 0));
			Model = scale(Model, vec3((1.0*scaleFactor), (1.0*scaleFactor), (1.0*scaleFactor)));
			//Model = translate(Model, vec3(10, 0, 0));
			MVP = Projection * View * Model;
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);// Send our transformation to the currently bound shader, in the "MVP" uniform
			renderAttrib(0, wallVertexBuffer); //Floor
			renderAttrib(1, leftColourBuffer); //Colours
			glDrawArrays(GL_TRIANGLES, 0, 2 * 3); //2*3 is 2 triangles of 3 vertices
		Model = popMat();

		//RIGHT WALL - BLUE
		pushMat(Model);
			Model = translate(Model, vec3(-(10*scaleFactor), 0, 0));
			Model = rotate(Model, 3.14159f / 2.0f, vec3(0, 1, 0));
			Model = rotate(Model, 3.14159f / 4.0f, vec3(1, 0, 0));
			Model = scale(Model, vec3((1.0*scaleFactor), (1.0*scaleFactor), (1.0*scaleFactor)));
			//Model = translate(Model, vec3(-10, 0, 0));
			MVP = Projection * View * Model;
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);// Send our transformation to the currently bound shader, in the "MVP" uniform
			renderAttrib(0, wallVertexBuffer); //Floor
			renderAttrib(1, rightColourBuffer); //Colours
			glDrawArrays(GL_TRIANGLES, 0, 2 * 3); //2*3 is 2 triangles of 3 vertices
		Model = popMat();

		//FAR WALL
		pushMat(Model);
			Model = translate(Model, vec3(0, 0, 10 * scaleFactor));
			Model = rotate(Model, 3.0f*3.14159f/4.0f, vec3(1, 0, 0));
			Model = scale(Model, vec3((1.0*scaleFactor), (1.0*scaleFactor), (1.0*scaleFactor)));
			MVP = Projection * View * Model;
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);// Send our transformation to the currently bound shader, in the "MVP" uniform
			renderAttrib(0, wallVertexBuffer); //Floor
			renderAttrib(1, farColourBuffer); //Colours
			glDrawArrays(GL_TRIANGLES, 0, 2 * 3); //2*3 is 2 triangles of 3 vertices
		Model = popMat();

		//NEAR WALL
		/*pushMat(Model);
			Model = translate(Model, vec3(0, 0, -10*scaleFactor));
			Model = rotate(Model, 3.14159f / 4.0f, vec3(1, 0, 0));
			MVP = Projection * View * Model;
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);// Send our transformation to the currently bound shader, in the "MVP" uniform
			renderAttrib(0, wallVertexBuffer); //Floor
			renderAttrib(1, nearColourBuffer); //Colours
			glDrawArrays(GL_TRIANGLES, 0, 2 * 3); //2*3 is 2 triangles of 3 vertices
		Model = popMat();*/

		/*glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);*/


		////****

		/*
		maipulator
		implement movement mechanism - so if have a given start point, and an end point, map the cube from the start to the end. So if hand moves from point a to point b, the cube then follows within the same frame.
		multiple transformations e.g. rotation too

		collision detection between multiple objects such as hand and cube
		*/

		//CUBE
		pushMat(Model);
			Model = rotpos;//MAY HAVE TO SCALE
			MVP = Projection * View * Model;
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
			renderAttrib(0, cubeVertexBuffer);
			renderAttrib(1, cubeColourBuffer);
			glDrawArrays(GL_TRIANGLES, 0, 12 * 3);
		Model = popMat();

		pushMat(Model);
			Model = rotpos2;//MAY HAVE TO SCALE
			MVP = Projection * View * Model;
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
			renderAttrib(0, cubeVertexBuffer);
			renderAttrib(1, cubeColourBuffer);
			glDrawArrays(GL_TRIANGLES, 0, 12 * 3);
		Model = popMat();
		
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		/*pushMat(Model);//why dont you work :(
			Model = translate(Model, vec3(2, 5, 2));
			MVP = Projection * View * Model;
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);// Send our transformation to the currently bound shader, in the "MVP" uniform
			renderAttrib(0, sphereVertexBuffer); //Floor
			renderAttrib(1, colorbuffer3); //Colours
			glDrawArrays(GL_TRIANGLES, 0, stacks*slices*3); //2*3 is 2 triangles of 3 vertices
		Model = popMat();

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);*/

		/*pushMat(Model);
			Model = translate(Model, vec3(0,5,0));
			MVP = Projection * View * Model;
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
			//renderScene();
			//glutSolidSphere(1.0, 20.0, 20.0);
		Model = popMat();



		glutDisplayFunc(renderScene);
		glutMainLoop();*/
		//glFlush();
		//glutMainLoop();
		// Swap buffers
		//glutSwapBuffers();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	//system("pause");

	//-------Clean up memory--------

	//OpenGL
	// Cleanup VBO and shader
	glDeleteBuffers(1, &cubeVertexBuffer);
	glDeleteBuffers(1, &wallVertexBuffer);
	glDeleteBuffers(1, &rightColourBuffer);
	glDeleteBuffers(1, &leftColourBuffer);
	glDeleteBuffers(1, &nearColourBuffer);
	glDeleteBuffers(1, &farColourBuffer);
	glDeleteBuffers(1, &cubeColourBuffer);
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	//Bullet
	dynamicsWorld->removeRigidBody(fallRigidBody);
	delete fallRigidBody->getMotionState();
	delete fallRigidBody;

	dynamicsWorld->removeRigidBody(groundRigidBody);
	delete groundRigidBody->getMotionState();
	delete groundRigidBody;


	delete fallShape;

	delete groundShape;

	delete dynamicsWorld;
	delete solver;
	delete dispatcher;
	delete collisionConfiguration;
	delete broadphase;

	return 0;
}

