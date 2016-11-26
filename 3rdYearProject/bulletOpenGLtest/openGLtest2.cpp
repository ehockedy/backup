//#include "stdafx.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#define GLEW_STATIC
#include <glew.h>
#include <glfw3.h>   //glfw-3.2.1.bin.WIN64/include/GLFW/glfw3.h> //Lib for 3D maths
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include "shader.hpp"
#include <btBulletDynamicsCommon.h>

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

// Initial position : on +Z
vec3 position = vec3(5, 5, 5);
// Initial horizontal angle : toward -Z
float horizontalAngle = -3.14f;// 3.14f / 2.0f;
// Initial vertical angle : none
float verticalAngle = 0.0f;
// Initial Field of View
float initialFoV = 45.0f;

float speed = 3.0f; // 3 units / second
float mouseSpeed = 0.005f;
int r = 10; //Distance from the cube

void computeMatricesFromInputs() { //CHANGE THIS SO THAT ONLY USE 6 KEYS FOR MOVING IN 6 DIRECTIONS

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
}

int main()
{
	//Initilize GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
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

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);//Set the background colour

	// Enable depth test - means closese things are draw over further away things
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	GLuint VertexArrayID; //A predefined OpenGL type to store an unsigned binary integer. This is different to an normal unsigned int in that it is a fixed size of 32 bis, as opposed to the size of a regular int whih can vary dependig on platform. This allows GL to work with the sizes it expects to.
	glGenVertexArrays(1, &VertexArrayID); //Specifies that 1 vertex array object name will be generated. 2nd arg is the array in which the generated vertex array object names are stored
	glBindVertexArray(VertexArrayID); //binds the vertex array object with name VertexArrayID

	GLuint programID = LoadShaders("vertexshader.txt", "fragmentshader.txt");

	//---------MATRICES---------
	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

	//PROJECTION matrix
	//Applies perspective to the scene i.e. objects further away appear smaller than objects that are closer
	//FoV = 45.0°
	//Aspect ration = width:height
	//Display range = 0.1 to 100 units
	Projection = perspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);

	//VIEW matrix (camera matrix)
	//Represents the movement of the virtual camera around the scene
	View = lookAt( //function to create a viewing matrix
		vec3(2, 2, 10), //Camera position in world coordinates
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

	static GLfloat g_vertex_buffer_data[] = {
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

	static GLfloat g_vertex_buffer_data2[] = {
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

	// This will identify our vertex buffer
	GLuint vertexbuffer;
	// Generate 1 buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &vertexbuffer);
	// The following commands will talk about our 'vertexbuffer' buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);


	//----------COLOURS----------
	// One color for each vertex. They were generated randomly.
	static GLfloat g_color_buffer_data[12*3*3];
	
	for (float i = 0.0; i < 12 * 3; i++)
	{
		g_color_buffer_data[(int)(3.0 * i + 0)] = fmod((i / 36), 1);
		g_color_buffer_data[(int)(3.0 * i + 1)] = fmod((i * 2) / 36, 1);
		g_color_buffer_data[(int)(3.0 * i + 2)] = fmod((i * 3) / 36, 1);
	}
	GLuint colorbuffer;
	glGenBuffers(1, &colorbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);

	//int j = 0;
	//int k = 0;
	

	//THE MAIN LOOP
	/*do {
		computeMatricesFromInputs();
		MVP = Projection * View * Model;

		//Clear the screen to the colour specified earlier, as well as the depth
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		// Send our transformation to the currently bound shader, in the "MVP" uniform
		//MatrixID - the location of the uniform value to be modified
		//1 - the number of elements that are to be modified. 1 means uniform variable is not an array, 1 or more if it is an array
		//&MVP[0][0] - pointer to an array of count variables that will be used to update the specified uniform variable
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		// 1st attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer); //This array is now accessable in the vertex shader
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // number of attributes (coordinates) per vertex
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);

		// 2nd attribute buffer : colors
		for (float i = 0.0; i < 12 * 3; i++)
		{
			g_color_buffer_data[(int)(3.0 * i + 0)] = fmod((i + j / 36), 1);
			g_color_buffer_data[(int)(3.0 * i + 1)] = fmod((i + j + 12) / 36, 1);
			g_color_buffer_data[(int)(3.0 * i + 2)] = fmod((i + j + 24) / 36, 1);
		}

		if (k == 0)
		{
			j++;
			j = j % 36;
		}

		k++;
		k = k % 36;

		glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);
		

		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
			);

		//Draw the cube
		glDrawArrays(GL_TRIANGLES, 0, 12*3); // Starting from vertex 0; 4 vertices total -> 1 triangle
		
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);*/


	//BULLET__________________________________________
	//First lines of any Bullet application
	//------------------
	btBroadphaseInterface* broadphase = new btDbvtBroadphase(); //Decide on the Broadphase algorithm - uses the bounding boxes of objects in the world to quickly compute a conservative approximate list of colliding pairs
	btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration(); //allows configuration of Bullet collision stack allocator and pool memory allocators
	btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);

	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver; //Solver - causes the objects to intersct properly, taking into account gravity, game logic supplied force, collisions, and hinge constraints

	btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration); //Instantiate dynamics world
	dynamicsWorld->setGravity(btVector3(0, -10, 0)); //Set gravity
													 //-----------------


	//COLLISION SHAPES - A collision shape determines collisions, it has no concept of mass inertia, etc. Many bodies can share a collision shape, but they should be the same shape.
	//Create ground shape
	btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 1); //Create a static rigid body that will be the ground plane. the y value is 1 because of another plane with y value -1 that will be created later on that will have the overall effect of y = 0. Couls also have the 2 planes have y = 0
	//Create sphere
	btCollisionShape* fallShape = new btSphereShape(1); //A sphere that will fall from the sky with a radius of 1


	//RIGID BODIES
	//Instantiate ground rigid body
	btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -1, 0))); //Instantiate the ground. Orientation is identity. Position is 1 meter below ground. This compensastes the 1m offset the shape itself had.
	//Combine shape and rigid body
	btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0)); //btRigidBodyConstructionInfo structure provides info to create a rigid body. First parameter is mass. When mass is zero, Bullet treats it as immovable i.e. infinite mass. 2nd and 3rd params are the motion shape (coupe of lienes above) and collision shape (in the shape section). Final param is inertia.
	//Create actual rigid body
	btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
	//Add groud to the world
	dynamicsWorld->addRigidBody(groundRigidBody);

	//Intantiate sphere rigid body
	btDefaultMotionState* fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 50, 0)));
	btScalar mass = 1; //Give it a mass of 1kg
	btVector3 fallInertia(0, 0, 0); //Initial inertia
	fallShape->calculateLocalInertia(mass, fallInertia); //Calculates the inertia as it falls
	btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState, fallShape, fallInertia); //If want to create lots of the bodies with the same parameters, only need to create one of these and pass that info to each body that is made
	btRigidBody* fallRigidBody = new btRigidBody(fallRigidBodyCI); //Create rigid body
	dynamicsWorld->addRigidBody(fallRigidBody); //Add to the world

	btScalar prevY = 0;
	//STEP THE SIMULATION
	//for (int i = 0; i < 300; i++)
	do
	{
		//BULLET STUFF
		dynamicsWorld->stepSimulation(1 / 3000.f, 10);

		btTransform trans; //Stores the state (position and orientation) of the falling sphere
		fallRigidBody->getMotionState()->getWorldTransform(trans);

		cout << "sphere height: " << trans.getOrigin().getY() << endl; //Only want the y position, got via getOrigin().getY()

		btScalar deltaY = trans.getOrigin().getY() - prevY;
		prevY = trans.getOrigin().getY();

		//OPENGL STUFF
		//Model = Model * translate(0.0f,0.0f,0.0f);
		computeMatricesFromInputs();
		MVP = Projection * View * Model;

		//Clear the screen to the colour specified earlier, as well as the depth
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		// Send our transformation to the currently bound shader, in the "MVP" uniform
		//MatrixID - the location of the uniform value to be modified
		//1 - the number of elements that are to be modified. 1 means uniform variable is not an array, 1 or more if it is an array
		//&MVP[0][0] - pointer to an array of count variables that will be used to update the specified uniform variable
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		/*
		maipulator
		implement movement mechanism - so if have a given start point, and an end point, map the cube from the start to the end. So if hand moves from point a to point b, the cube then follows within the same frame.
		multiple transformations e.g. rotation too

		collision detection between multiple objects such as hand and cube
		*/

		// 1st attribute buffer : vertices
		
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer); //This array is now accessable in the vertex shader

		for (int i = 1; i < numVertices; i=i+3) //For y value in each vertex
		{
			g_vertex_buffer_data[i] = trans.getOrigin().getY() + g_vertex_buffer_data2[i];
		}
		
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		// Give our vertices to OpenGL.
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

		
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // number of attributes (coordinates) per vertex
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);



		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);

		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
			);

		//Draw the cube
		glDrawArrays(GL_TRIANGLES, 0, 12 * 3); // Starting from vertex 0; 4 vertices total -> 1 triangle

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	//system("pause");

	//-------Clean up memory--------

	//OpenGL
	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &colorbuffer);
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

