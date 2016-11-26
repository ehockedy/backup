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

int main2()
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

	GLint width = 1024;
	GLint height = 768;
	// Open a window and create its OpenGL context
	GLFWwindow* window; // (In the accompanying source code, this variable is global)
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

	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);//Set the background colour

	GLuint VertexArrayID; //A predefined OpenGL type to store an unsigned binary integer. This is different to an normal unsigned int in that it is a fixed size of 32 bis, as opposed to the size of a regular int whih can vary dependig on platform. This allows GL to work with the sizes it expects to.
	glGenVertexArrays(1, &VertexArrayID); //Specifies that 1 vertex array object name will be generated. 2nd arg is the array in which the generated vertex array object names are stored
	glBindVertexArray(VertexArrayID); //binds the vertex array object with name VertexArrayID

	GLuint programID = LoadShaders("vertexshader.txt", "fragmentshader.txt");

	//MATRICES
	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

	//PROJECTION matrix
	//Applies perspective to the scene i.e. objects further away appear smaller than objects that are closer
	//FoV = 45.0°
	//Aspect ration = width:height
	//Display range = 0.1 to 100 units
	mat4 Projection = perspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);

	//VIEW matrix (camera matrix)
	//Represents the movement of the virtual camera around the scene
	mat4 View = lookAt( //function to create a viewing matrix
		vec3(1, 2, 10), //Camera position in world coordinates
		vec3(0, 0, 0), //Where the camera looks. Having the x and y coor from the cam pos and then negative (or any smaller) z value makes it look straight down the z axis if no transformations are being applied.
		vec3(0, 1, 0) //Head is up ((0,-1,0) looks upside down)
		);

	/*MODEL matrix
	The model matrix. Matrices can represent translations, rotations, and other geometrical transformations. You can use a single 4×4 matrix to represent any number of 
	transformations in 3D space; you start with the identity matrix — that is, the matrix that represents a transformation that does nothing at all — then 
	multiply it by the matrix that represents your first transformation, then by the one that represents your second transformation, and so on. The 
	combined matrix represents all of your transformations in one. The matrix we use to represent this current move/rotate state is called the model matrix*/
	mat4 Model = mat4({ 1.0f, 0.0f, 0.0f, 2.0f, 
						0.0f, 1.0f, 0.0f, 0.0f,
						0.0f, 0.0f, 1.0f, 0.0f,
						0.0f, 0.0f, 0.0f, 1.0f }); //An identity matrix

	//MODEL VIEW PROJECTION matrix
	mat4 MVP = Projection * View * Model; //Remember matrix multiplication applies transformations from right to left



	// An array of  vectors which represents 4 vertices
	//x direction is left to right
	//y direction is down to up
	//z direction is behind to in front
	//In each direction, coordinates go from -1 to 1 i.e. this triangle will take up most of the screen
	static const GLfloat g_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f, 
		1.0f, -1.0f, 0.0f,
		1.0f,  1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
	};

	// This will identify our vertex buffer
	GLuint vertexbuffer;
	// Generate 1 buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &vertexbuffer);
	// The following commands will talk about our 'vertexbuffer' buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);


	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	//THE MAIN LOOP
	do {
		//Clear the screen to the colour specified earlier
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		// Send our transformation to the currently bound shader, in the "MVP" uniform
		//MatrixID - the location of the uniform value to be modified
		//1 - the number of elements that are to be modified. 1 means uniform variable is not an array, 1 or more if it is an array
		//&MVP[0][0] - pointer to an array of count variables that will be used to update the specified uniform variable
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // number of attributes (coordinates) per vertex
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);
		// Draw the triangle !
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4); // Starting from vertex 0; 4 vertices total -> 1 triangle
		glDisableVertexAttribArray(0);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);


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


	//STEP THE SIMULATION
	for (int i = 0; i < 300; i++)
	{

		dynamicsWorld->stepSimulation(1 / 60.f, 10);

		btTransform trans; //Stores the state (position and orientation) of the falling sphere
		fallRigidBody->getMotionState()->getWorldTransform(trans);

		cout << "sphere height: " << trans.getOrigin().getY() << endl; //Only want the y position, got via getOrigin().getY()
	}

	//system("pause");

	//-------Clean up memory--------

	//OpenGL
	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
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