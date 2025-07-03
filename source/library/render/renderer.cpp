#include "renderer.h"

#include "include_opengl.h"

#include "shaders/shaders.h"
#include "shaders/shaders_compilation.h"

#include <iostream>
#include <exception>

constexpr float VERTICES[] = {
		 0.0f,  0.5f,
		-0.5f, -0.5f,
		 0.5f, -0.5f
};

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
	m_vertex_shader_object_id = shaders_compilation::compile_shader(gl::GL_VERTEX_SHADER, VERTEX_SHADER);
	m_fragment_shader_id = shaders_compilation::compile_shader(gl::GL_FRAGMENT_SHADER, FRAGMENT_SHADER);
	shader_program_id = shaders_compilation::link_shaders_to_program(m_vertex_shader_object_id, m_fragment_shader_id);
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
