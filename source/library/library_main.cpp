#include "library_main.h"

#include <gl2platform.h>

#include <iostream>
#include <exception>


static constexpr char* VERTEX_SHADER = R"(
#version 330 core // TODO get this to be OpenGL SC 2.0 complient, GPT4 generated.
layout (location = 0) in vec2 aPos;
uniform float angle;
void main() {
    float s = sin(angle);
    float c = cos(angle);
    mat2 rotation = mat2(c, -s, s, c);
    gl_Position = vec4(rotation * aPos, 0.0, 1.0);
}
)";

static constexpr char* FRAGMENT_SHADER = R"(
#version 330 core // TODO get this to be OpenGL SC 2.0 complient, GPT4 generated.
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0, 0.5, 0.2, 1.0);
}
)";

constexpr float VERTICES[] = {
		 0.0f,  0.5f,
		-0.5f, -0.5f,
		 0.5f, -0.5f
};

// public
/////////

void library_main::run()
{
	initialise();
	static float angle = 0.0f;
	constexpr float speed = 0.1f;
	static float running_time = 0.0f;
	while (!glfwWindowShouldClose(m_window))
	{
		// todo add basic rotating triangle.
		const float delta_time = static_cast<float>(glfwGetTime()) - running_time;
		running_time += delta_time;
		angle += speed * delta_time;



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
	initialise_shaders();
}

void library_main::initialise_shaders()
{

}

void library_main::shutdown()
{
	if (m_window)
	{
		glfwDestroyWindow(m_window);
		m_window = nullptr;
	}
	glfwTerminate();
}