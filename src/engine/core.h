#ifndef __CORE__
#define __CORE__

#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifdef _WIN32
#define ASSERT(x) \
	if (!(x))     \
		__debugbreak();
#else
#define ASSERT(x) \
	if (!(x))     \
		__builtin_trap();
#endif

#define GL_CHECK(x) \
	GLclearError(); \
	x;              \
	ASSERT(GLlogCall(#x, __FILE__, __LINE__))
#define DEBUG_LOG(msg)                 \
	{                                  \
		std::cout << msg << std::endl; \
	}
#define ERR_LOG(msg)                   \
	{                                  \
		std::cerr << msg << std::endl; \
	}
#define GLFW_CHECK() GLFW_check_error();

void GLFW_check_error();
void GLclearError();
bool GLlogCall(const char *function, const char *file, int line);

#endif