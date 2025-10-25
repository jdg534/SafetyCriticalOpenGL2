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
	m_render_list.reserve(m_render_list_cap);
}

void renderer::shutdown()
{
	m_render_list.clear();
	shutdown_shaders();
}

void renderer::render_frame()
{
	using namespace gl;
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
	sort(begin(m_render_list), end(m_render_list),
		// < operator logic
		[&](const weak_ptr<renderable>& a, const weak_ptr<renderable>& b)
		{
			const renderable_type a_type = a.lock()->get_renderable_type();
			const renderable_type b_type = b.lock()->get_renderable_type();
			if (a_type != b_type) { return a_type < b_type; }
			return false;
		});
}

void renderer::set_framebuffer_size(glm::vec2 framebuffer_size)
{
	m_framebuffer_size = framebuffer_size;
}

// private
//////////

void renderer::initialise_shaders()
{
	m_static_geometry_vertex_shader_object_id = shaders_compilation::compile_shader(gl::GL_VERTEX_SHADER, STATIC_MESH_VERTEX_SHADER);
	m_static_geometry_fragment_shader_id = shaders_compilation::compile_shader(gl::GL_FRAGMENT_SHADER, STATIC_MESH_FRAGMENT_SHADER);
	m_static_geometry_program_id = shaders_compilation::link_shaders_to_program(m_static_geometry_vertex_shader_object_id, m_static_geometry_fragment_shader_id);

	m_textured_quad_geometry_vertex_shader_object_id = shaders_compilation::compile_shader(gl::GL_VERTEX_SHADER, TEXTURED_QUAD_VERTEX_SHADER);
	m_textured_quad_geometry_fragment_shander_id = shaders_compilation::compile_shader(gl::GL_FRAGMENT_SHADER, TEXTURED_QUAD_FRAGMENT_SHADER);
	m_textured_quad_geometry_program_id = shaders_compilation::link_shaders_to_program(m_textured_quad_geometry_vertex_shader_object_id, m_textured_quad_geometry_fragment_shander_id);
}

void renderer::shutdown_shaders()
{
	// When adding 3d rendering shutdown the shaders here.

	gl::glDeleteShader(m_static_geometry_vertex_shader_object_id);
	gl::glDeleteShader(m_static_geometry_fragment_shader_id);
	gl::glDeleteProgram(m_static_geometry_program_id);
	gl::glDeleteShader(m_textured_quad_geometry_vertex_shader_object_id);
	gl::glDeleteShader(m_textured_quad_geometry_fragment_shander_id);
	gl::glDeleteProgram(m_textured_quad_geometry_program_id);
}

void renderer::switch_to_3d_static_mesh_shader()
{
	if (m_current_shader_program == m_static_geometry_program_id) return;
	gl::glUseProgram(m_static_geometry_program_id);
	m_current_shader_program = m_static_geometry_program_id;
	gl::glEnable(gl::GL_DEPTH_TEST);
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
	gl::glEnable(gl::GL_BLEND);
	gl::glBlendFunc(gl::GL_SRC_ALPHA, gl::GL_ONE_MINUS_SRC_ALPHA);
	gl::glDisable(gl::GL_DEPTH_TEST);
}
