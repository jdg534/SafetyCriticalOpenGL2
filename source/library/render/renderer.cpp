#include "renderer.h"

#include "include_opengl.h"

#include "shaders/shaders.h"
#include "shaders/shaders_compilation.h"

#include "renderable.h"

#include <glbinding/gl/types.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.inl>

#include <algorithm>
#include <iostream>
#include <exception>

constexpr float VERTICES[] = {
		 0.0f,  0.5f,
		-0.5f, -0.5f,
		 0.5f, -0.5f
};

// public
/////////

renderer::renderer(glm::vec2 framebuffer_size, const size_t render_list_cap)
	: m_framebuffer_size(framebuffer_size)
	, m_render_list_cap(render_list_cap)
{

}

void renderer::initialise()
{
	initialise_shaders();
	initialise_object_buffers(); // refactor this out when getting to having different objects.
	m_render_list.reserve(m_render_list_cap);
}

void renderer::shutdown()
{
	m_render_list.clear();
	shutdown_shaders();
	shutdown_object_buffers();
}

void renderer::render_frame()
{
	static float angle = 0.0f; // refactor the placeholder out.
	gl::glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	gl::glClear(gl::GL_COLOR_BUFFER_BIT);

	// <place holder code>
	gl::glUseProgram(shader_program_id); // shader specfic
	m_current_shader_program = shader_program_id;
	gl::GLint angle_loc = gl::glGetUniformLocation(shader_program_id, "angle");
	gl::glUniform1f(angle_loc, angle);

	gl::glBindVertexArray(m_vertex_arrary_object_id); // drawable specfic
	gl::glDrawArrays(gl::GL_TRIANGLES, 0, 3);

	angle += 0.01f;
	// </place holder code>

	
	for (auto to_draw_iter : m_render_list)
	{
		auto to_draw = to_draw_iter.lock();
		switch (to_draw->get_renderable_type())
		{
			case renderable_type::STATIC_GEOMETRY:
				switch_to_3d_static_mesh_shader();
				break;
			case renderable_type::_2D_GEOMETRY:
				switch_to_2d_shader();
				break;
			default:
				throw std::exception("attempted to render unsupported type");
				break;
		}
		to_draw->draw();
	}
}

void renderer::add_to_render_list(std::weak_ptr<renderable> to_add)
{
	if (m_render_list.size() < m_render_list_cap)
	{
		m_render_list.push_back(to_add);
		auto to_add_to_mod = to_add.lock();
		switch (to_add_to_mod->get_renderable_type())
		{
			case renderable_type::STATIC_GEOMETRY:
				to_add_to_mod->set_shader_program(m_static_geometry_program_id);
				break;
			case renderable_type::_2D_GEOMETRY:
				to_add_to_mod->set_shader_program(m_textured_quad_geometry_program_id);
				break;
			default:
				throw std::exception("attempted to add unsupported renderable type");
				break;
		}
	}
	else
	{
		throw std::exception("Attempting to add too much to the render list");
	}
}

void renderer::sort_render_list()
{
	using namespace std;
	// changing shaders and texutes is expencive, sort to minimise the amount of setting needed.
	m_index_of_first_3d_static_mesh = m_index_of_first_2d_renderable = -1;
	// std::sort
	sort(begin(m_render_list), end(m_render_list),
		// < operator logic
		[&](const weak_ptr<renderable>& a, const weak_ptr<renderable>& b)
		{
			const renderable_type a_type = a.lock()->get_renderable_type();
			const renderable_type b_type = b.lock()->get_renderable_type();
			if (a_type != b_type) { return a_type < b_type; }
			return false;
		});

	// std::first of dynamic cast 3d
	// std::first of dynamic cast 2d
	m_index_of_first_3d_static_mesh = m_index_of_first_2d_renderable = -1;
	const auto first_3d_iter = find_if(cbegin(m_render_list), cend(m_render_list), [](const weak_ptr<renderable>& a)
		{ return a.lock()->get_renderable_type() == renderable_type::STATIC_GEOMETRY; });
	const auto first_2d_iter = find_if(cbegin(m_render_list), cend(m_render_list), [](const weak_ptr<renderable>& a)
		{ return a.lock()->get_renderable_type() == renderable_type::_2D_GEOMETRY; });
	if (first_3d_iter != cend(m_render_list)) { m_index_of_first_3d_static_mesh = distance(cbegin(m_render_list), first_3d_iter); }
	if (first_2d_iter != cend(m_render_list)) { m_index_of_first_2d_renderable = distance(cbegin(m_render_list), first_2d_iter); }
}

void renderer::set_framebuffer_size(glm::vec2 framebuffer_size)
{
	m_framebuffer_size = framebuffer_size;
}

// private
//////////

void renderer::initialise_shaders()
{
	m_vertex_shader_object_id = shaders_compilation::compile_shader(gl::GL_VERTEX_SHADER, VERTEX_SHADER);
	m_fragment_shader_id = shaders_compilation::compile_shader(gl::GL_FRAGMENT_SHADER, FRAGMENT_SHADER);
	shader_program_id = shaders_compilation::link_shaders_to_program(m_vertex_shader_object_id, m_fragment_shader_id);

	m_static_geometry_vertex_shader_object_id = shaders_compilation::compile_shader(gl::GL_VERTEX_SHADER, STATIC_MESH_VERTEX_SHADER);
	m_static_geometry_fragment_shader_id = shaders_compilation::compile_shader(gl::GL_FRAGMENT_SHADER, STATIC_MESH_FRAGMENT_SHADER);
	m_static_geometry_program_id = shaders_compilation::link_shaders_to_program(m_static_geometry_vertex_shader_object_id, m_static_geometry_fragment_shader_id);

	m_textured_quad_geometry_vertex_shader_object_id = shaders_compilation::compile_shader(gl::GL_VERTEX_SHADER, TEXTURED_QUAD_VERTEX_SHADER);
	m_textured_quad_geometry_fragment_shander_id = shaders_compilation::compile_shader(gl::GL_FRAGMENT_SHADER, TEXTURED_QUAD_FRAGMENT_SHADER);
	m_textured_quad_geometry_program_id = shaders_compilation::link_shaders_to_program(m_textured_quad_geometry_vertex_shader_object_id, m_textured_quad_geometry_fragment_shander_id);
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
	// the above are placeholders. delete them when both 3d geometry and textured quads can be drawn.

	gl::glDeleteShader(m_static_geometry_vertex_shader_object_id);
	gl::glDeleteShader(m_static_geometry_fragment_shader_id);
	gl::glDeleteProgram(m_static_geometry_program_id);
	gl::glDeleteShader(m_textured_quad_geometry_vertex_shader_object_id);
	gl::glDeleteShader(m_textured_quad_geometry_fragment_shander_id);
	gl::glDeleteProgram(m_textured_quad_geometry_program_id);
}

void renderer::shutdown_object_buffers()
{
	gl::glDeleteBuffers(1, &m_vertex_buffer_object_id);
	gl::glDeleteVertexArrays(1, &m_vertex_arrary_object_id);
}

void renderer::switch_to_3d_static_mesh_shader()
{
	if (m_current_shader_program == m_static_geometry_program_id) return;
	gl::glUseProgram(m_static_geometry_program_id);
	m_current_shader_program = m_static_geometry_program_id;

	// TODO: code this!
	// set the uniforms. as they appear in: source/library/render/shaders/static_mesh_shader.h
}

void renderer::switch_to_2d_shader()
{
	if (m_current_shader_program == m_textured_quad_geometry_program_id) return;
	gl::glUseProgram(m_textured_quad_geometry_program_id);
	m_current_shader_program = m_textured_quad_geometry_program_id;

	// set the uniforms (renderer level). as they appear in: source/library/render/shaders/textured_quad_shader.h
	// other uniforms are to be set by the renderable object.
	const gl::GLint u_resolution_location = gl::glGetUniformLocation(m_textured_quad_geometry_program_id, "u_resolution");
	if (u_resolution_location != -1)
	{
		gl::glUniform2fv(u_resolution_location, 1, glm::value_ptr(m_framebuffer_size));
	}
}
