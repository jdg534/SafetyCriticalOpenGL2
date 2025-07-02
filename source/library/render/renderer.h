#pragma once

#include "include_opengl.h"

#include <glbinding/gl/types.h>

class renderer
{
public:

	renderer() = delete;
	explicit renderer(GLFWwindow* window);

	void initialise();
	void shutdown();

	void render_frame();

private:

	void initialise_shaders();
	void initialise_object_buffers();

	void shutdown_shaders();
	void shutdown_object_buffers();

	// refactor these out. Get bitmap font rendering working as the case for using different glsl programs.
	gl::GLuint m_vertex_shader_object_id = 0, m_fragment_shader_id = 0, shader_program_id = 0;
	gl::GLuint m_vertex_arrary_object_id = 0, m_vertex_buffer_object_id = 0, m_vertex_attribute_object_id = 0;

	const GLFWwindow* m_window { nullptr };
};
