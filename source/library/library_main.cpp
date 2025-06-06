#include "library_main.h"

#include <iostream>

#include <gl2platform.h>

#include <exception>

// public
/////////

void library_main::run()
{
	initialise();
	while (!glfwWindowShouldClose(m_window))
	{
		// todo add basic rotating triangle.
		glfwPollEvents();
	}
	shutdown();
}

// private
//////////

void library_main::initialise()
{
	if (glfwInit() != GLFW_TRUE)
	{
		throw std::exception("glfwInit() failed");
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // todo figure out correct values of SC 2.0
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // ''
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // ''

	m_window = glfwCreateWindow(800, 600, "A window", nullptr, nullptr);
	if (m_window == nullptr)
	{
		throw std::exception("glfwCreateWindow() failed");
	}
}

void library_main::shutdown()
{
	if (m_window)
	{
		glfwDestroyWindow(m_window);
		m_window = nullptr;
	}
}