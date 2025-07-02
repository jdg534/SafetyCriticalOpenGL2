#include "library_main.h"

#include "render/renderer.h"

#include "render/include_opengl.h"

#include <iostream>
#include <exception>

// public
/////////

void library_main::run()
{
	initialise();
	static float running_time = 0.0f;
	while (!glfwWindowShouldClose(m_window))
	{
		const float delta_time = static_cast<float>(glfwGetTime()) - running_time;
		running_time += delta_time;
		m_renderer->render_frame();
		glfwSwapBuffers(m_window);
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
		const char* error_str[256];
		error_str[0] = '\0';
		const int error_code = glfwGetError(error_str);
		std::cerr << "glfwCreateWindow() failed: " << *error_str << std::endl;
		throw std::exception("glfwCreateWindow() failed");
	}
	glfwMakeContextCurrent(m_window);
	glbinding::initialize(glfwGetProcAddress);
	m_renderer = new renderer(m_window);
	m_renderer->initialise();
}

void library_main::shutdown()
{
	if (m_renderer)
	{
		m_renderer->shutdown();
		delete m_renderer;
		m_renderer = nullptr;
	}
	if (m_window)
	{
		glfwDestroyWindow(m_window);
		m_window = nullptr;
	}
	glfwTerminate();
}
