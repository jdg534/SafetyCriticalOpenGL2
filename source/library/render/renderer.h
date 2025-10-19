#pragma once

#include "include_opengl.h"

#include <glbinding/gl/types.h>
#include <glm/glm.hpp>

#include <memory>
#include <vector>

// todo add a foward declare header for all struct and class types.
class renderable;

class renderer
{
public:

	renderer() = delete;
	explicit renderer(glm::vec2 framebuffer_size, const size_t render_list_cap);

	void initialise();
	void shutdown();

	void render_frame();

	void add_to_render_list(std::weak_ptr<renderable> to_add);
	// todo(if needed): remove_from_render_list()
	void sort_render_list();

	void set_framebuffer_size(glm::vec2 framebuffer_size);

private:

	void initialise_shaders();
	void initialise_object_buffers();

	void shutdown_shaders();
	void shutdown_object_buffers();

	void switch_to_3d_static_mesh_shader();
	void switch_to_2d_shader();

	gl::GLuint m_vertex_shader_object_id = 0,
		m_fragment_shader_id = 0,
		shader_program_id = 0; // placeholder, delete later.
	gl::GLuint m_static_geometry_vertex_shader_object_id = 0,
		m_static_geometry_fragment_shader_id = 0,
		m_static_geometry_program_id = 0;
	gl::GLuint m_textured_quad_geometry_vertex_shader_object_id = 0,
		m_textured_quad_geometry_fragment_shander_id = 0,
		m_textured_quad_geometry_program_id = 0;

	// placeholders, move them somewhere more sensible later.
	gl::GLuint m_vertex_arrary_object_id = 0, m_vertex_buffer_object_id = 0, m_vertex_attribute_object_id = 0;

	gl::GLuint m_current_shader_program = 0;
	int m_index_of_first_3d_static_mesh = -1;
	int m_index_of_first_2d_renderable = -1;

	glm::vec2 m_framebuffer_size;

	std::vector<std::weak_ptr<renderable>> m_render_list;
	const size_t m_render_list_cap { 0 };
};
