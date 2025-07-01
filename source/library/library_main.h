#pragma once
// HI C++ might not accept this.if so go to #ifndef #define #endif

#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h> // include order dependent... >:(
#include <GLFW/glfw3.h>

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


	GLuint m_vertex_buffer_object_id = 0, m_vertex_attribute_object_id = 0;

	GLFWwindow* m_window { nullptr };
};

