#include "renderer.h"

// start of stuff to put in include_opengl.h
#define GLFW_INCLUDE_NONE // Prevents GLFW from including OpenGL headers
#include <GLFW/glfw3.h>
#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h> // include order dependent... >:( TODO: add a include_opengl.h
//#include <gl2platform.h>
// end of stuff to put in include_opengl.h

#include <iostream>
#include <exception>

static constexpr char* VERTEX_SHADER = R"(
#version 330 core
// TODO get this to be OpenGL SC 2.0 complient, GPT4 generated.
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
#version 330 core
// TODO get this to be OpenGL SC 2.0 complient, GPT4 generated.
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

gl::GLuint compile_shader(gl::GLenum type, const char* src) {
	gl::GLuint s = gl::glCreateShader(type);
	gl::glShaderSource(s, 1, &src, nullptr);
	gl::glCompileShader(s);
	gl::GLint ok = 0;
	gl::glGetShaderiv(s, gl::GL_COMPILE_STATUS, &ok);
	if (!ok)
	{
		char log[512];
		std::memset(log, 0, 512);
		gl::glGetShaderInfoLog(s, 512, nullptr, log);
		std::cerr << "Shader compile error:\n" << log << std::endl;
		std::exit(EXIT_FAILURE);
	}
	return s;
}

gl::GLuint link_shaders_to_program(gl::GLuint vertex_shader_id, gl::GLuint fragment_shader_id)
{
	gl::GLuint p = gl::glCreateProgram();
	gl::glAttachShader(p, vertex_shader_id);
	gl::glAttachShader(p, fragment_shader_id);
	gl::glLinkProgram(p);
	gl::GLint ok = 0;
	gl::glGetProgramiv(p, gl::GL_LINK_STATUS, &ok);
	if (!ok)
	{
		char log[512];
		std::memset(log, 0, 512);
		gl::glGetProgramInfoLog(p, 512, nullptr, log);
		std::cerr << "Program link error:\n" << log << std::endl;
		std::exit(EXIT_FAILURE);
	}
	return p;
}

// public
/////////

renderer::renderer(GLFWwindow* window)
	: m_window(window)
{

}


void renderer::initialise()
{
	initialise_shaders();
	initialise_object_buffers(); // refactor this out when getting to having different objects.
}

void renderer::shutdown()
{
	shutdown_shaders();
	shutdown_object_buffers();
}

void renderer::render_frame()
{
	static float angle = 0.0f; // refactor the placeholder out.
	gl::glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	gl::glClear(gl::GL_COLOR_BUFFER_BIT);

	gl::glUseProgram(shader_program_id); // shader specfic
	gl::GLint angle_loc = gl::glGetUniformLocation(shader_program_id, "angle");
	gl::glUniform1f(angle_loc, angle);

	gl::glBindVertexArray(m_vertex_arrary_object_id); // drawable specfic
	gl::glDrawArrays(gl::GL_TRIANGLES, 0, 3);

	// placeholder code. remove when decoupled object drawing object from render code.
	angle += 0.01f;
}

// private
//////////

void renderer::initialise_shaders()
{
	m_vertex_shader_object_id = compile_shader(gl::GL_VERTEX_SHADER, VERTEX_SHADER);
	m_fragment_shader_id = compile_shader(gl::GL_FRAGMENT_SHADER, FRAGMENT_SHADER);
	shader_program_id = link_shaders_to_program(m_vertex_shader_object_id, m_fragment_shader_id);
}

void renderer::initialise_object_buffers()
{
	gl::glGenVertexArrays(1, &m_vertex_arrary_object_id);
	gl::glGenBuffers(1, &m_vertex_buffer_object_id);

	gl::glBindVertexArray(m_vertex_arrary_object_id);
	gl::glBindBuffer(gl::GL_ARRAY_BUFFER, m_vertex_buffer_object_id);
	gl::glBufferData(gl::GL_ARRAY_BUFFER, sizeof(VERTICES), VERTICES, gl::GL_STATIC_DRAW); // refactor!
	gl::glVertexAttribPointer(0, 2, gl::GL_FLOAT, gl::GL_FALSE, 2 * sizeof(float), nullptr);
	gl::glEnableVertexAttribArray(0);
}


void renderer::shutdown_shaders()
{
	gl::glDeleteShader(m_vertex_shader_object_id);
	gl::glDeleteShader(m_fragment_shader_id);
	gl::glDeleteProgram(shader_program_id);
}

void renderer::shutdown_object_buffers()
{
	gl::glDeleteBuffers(1, &m_vertex_buffer_object_id);
	gl::glDeleteVertexArrays(1, &m_vertex_arrary_object_id);
}
