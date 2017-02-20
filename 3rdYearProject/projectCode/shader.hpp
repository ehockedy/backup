//#define GLEW_STATIC
#include <glew.h>
//#include <glfw3.h>   //glfw-3.2.1.bin.WIN64/include/GLFW/glfw3.h> //Lib for 3D maths
//#include <glm.hpp>

#ifndef SHADER_HPP
#define SHADER_HPP

GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path);

#endif
