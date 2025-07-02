#pragma once
// HI C++ might not accept this.if so go to #ifndef #define #endif

#include <glbinding/gl/types.h>

struct GLFWwindow;

// class for 
class library_main
{
public:

	void run();

private:

	void initialise();
	void initialise_shaders();
	void initialise_object_buffers();

	void shutdown();
	void shutdown_shaders();
	void shutdown_object_buffers();


	gl::GLuint m_vertex_shader_object_id = 0, m_fragment_shader_id = 0, shader_program_id = 0;
	gl::GLuint m_vertex_arrary_object_id = 0, m_vertex_buffer_object_id = 0, m_vertex_attribute_object_id = 0;

	GLFWwindow* m_window { nullptr };
};

