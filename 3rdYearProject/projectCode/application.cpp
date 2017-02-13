//#pragma once
//#include "stdafx.h"
/*#include <iostream>
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
#include <vector>*/
//#include "application.h"

//using namespace std;
//using namespace glm;


/*int main(int argc, char** argv) {
	
	App a;
	a.setup();
	Cube cube1 = Cube(1, 0, 0);
	

	do {
		a.preprocessing();

		pushMat(Model);
			//Model = rotpos2;//MAY HAVE TO SCALE
			MVP = Projection * View * Model;
			glUniformMatrix4fv(a.getMatrixID(), 1, GL_FALSE, &MVP[0][0]);
			renderAttrib(0, *cube1.getVertexBuffer());
			renderAttrib(1, *cube1.getColourBuffer());
			glDrawArrays(GL_TRIANGLES, 0, 12 * 3);
		Model = popMat();

		a.postprocessing();
	}
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);
}*/