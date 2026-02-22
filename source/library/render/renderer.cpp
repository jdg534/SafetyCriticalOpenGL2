#include "renderer.h"

#include "include_opengl.h"

#include "shaders/shaders.h"
#include "shaders/shaders_compilation.h"

#include "renderable.h"
#include "3d/camera.h"

#include <glbinding/gl/types.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.inl>

#include <algorithm>
#include <iostream>
#include <exception>

// public
/////////

renderer::renderer(glm::vec2 framebuffer_size, const size_t render_list_cap, std::weak_ptr<const camera> camera)
	: m_framebuffer_size(framebuffer_size)
	, m_render_list_cap(render_list_cap)
	, m_camera(camera)
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
	
	glClearColor(m_clear_colour.r, m_clear_colour.g, m_clear_colour.b, m_clear_colour.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const render_mode current_render_mode = get_render_mode();
	switch (current_render_mode)
	{
		case render_mode::POINTS: glPolygonMode(GL_FRONT_AND_BACK, GL_POINTS); break;
		case render_mode::LINES: glPolygonMode(GL_FRONT_AND_BACK, GL_LINES); break;
		case render_mode::FILL: glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); break;
		default: break;
	}

	for (auto to_draw_iter : m_render_list)
	{
		auto to_draw = to_draw_iter.lock();
		switch (to_draw->get_renderable_type())
		{
			case renderable_type::TERRAIN: switch_to_terrain_shader(); break;
			case renderable_type::STATIC_GEOMETRY: switch_to_3d_static_mesh_shader(); break;
			case renderable_type::_2D_GEOMETRY: switch_to_2d_shader(); break;
			default:
				throw std::exception("attempted to render unsupported type"); break;
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
			case renderable_type::TERRAIN: to_add_to_mod->set_shader_program(m_terrain_program_id); break;
			case renderable_type::STATIC_GEOMETRY: to_add_to_mod->set_shader_program(m_static_geometry_program_id); break;
			case renderable_type::_2D_GEOMETRY: to_add_to_mod->set_shader_program(m_textured_quad_geometry_program_id); break;
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

void renderer::set_camera(std::weak_ptr<const camera> camera)
{
	m_camera = camera;
}

glm::vec4 renderer::get_clear_colour() const
{
	return m_clear_colour;
}

glm::vec3 renderer::get_ambient_light_colour() const
{
	return m_ambient_light_colour;
}

glm::vec3 renderer::get_directional_light_colour() const
{
	return m_directional_light_colour;
}

glm::vec3 renderer::get_directional_light_direction() const
{
	return m_directional_light_direction;
}

render_mode renderer::get_render_mode() const
{
	return m_render_mode;
}

void renderer::set_clear_colour(glm::vec4 colour)
{
	m_clear_colour = colour;
}

void renderer::set_ambient_light_colour(glm::vec3 colour)
{
	m_ambient_light_colour = colour;
}

void renderer::set_directional_light_colour(glm::vec3 colour)
{
	m_directional_light_colour = colour;
}

void renderer::set_directional_light_direction(glm::vec3 direction)
{
	m_directional_light_direction = direction;
}

void renderer::set_render_mode(render_mode render_mode)
{
	m_render_mode = render_mode;
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

	m_terrain_vertex_shader_object_id = shaders_compilation::compile_shader(gl::GL_VERTEX_SHADER, TERRAIN_VERTEX_SHADER);
	m_terrain_fragment_shander_id = shaders_compilation::compile_shader(gl::GL_FRAGMENT_SHADER, TERRAIN_FRAGMENT_SHADER);
	m_terrain_program_id = shaders_compilation::link_shaders_to_program(m_terrain_vertex_shader_object_id, m_terrain_fragment_shander_id);
}

void renderer::shutdown_shaders()
{
	using namespace gl;

	for (GLuint shader_id : {m_static_geometry_vertex_shader_object_id, m_static_geometry_fragment_shader_id,
		m_textured_quad_geometry_vertex_shader_object_id, m_textured_quad_geometry_fragment_shander_id,
		m_terrain_vertex_shader_object_id, m_terrain_fragment_shander_id})
	{
		glDeleteShader(shader_id);
	}
	for (GLuint program_id : {m_textured_quad_geometry_program_id, m_static_geometry_program_id, m_terrain_program_id})
	{
		glDeleteProgram(program_id);
	}
}

void renderer::switch_to_terrain_shader()
{
	using namespace gl;
	if (m_current_shader_program == m_terrain_program_id) return;
	glUseProgram(m_terrain_program_id);
	m_current_shader_program = m_terrain_program_id;
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);   // LH system requires clockwise winding, CW = Clock Wise winding order.
	glCullFace(GL_BACK);

	// set the uniforms (renderer level). as they appear in: source/library/render/shaders/terrain_shader.h
	// other uniforms are to be set by the renderable object.
	const GLint u_view_loc = glGetUniformLocation(m_terrain_program_id, "u_view");
	const GLint u_projection_loc = glGetUniformLocation(m_terrain_program_id, "u_projection");
	const GLint u_light_direction_loc = glGetUniformLocation(m_terrain_program_id, "u_light_direction");
	const GLint u_light_colour_loc = glGetUniformLocation(m_terrain_program_id, "u_light_colour");
	const GLint u_ambient_light_colour_loc = glGetUniformLocation(m_terrain_program_id, "u_ambient_light_colour");

	glUniformMatrix4fv(u_view_loc, 1, GL_FALSE, glm::value_ptr(m_camera.lock()->get_view_matrix()));
	glUniformMatrix4fv(u_projection_loc, 1, GL_FALSE, glm::value_ptr(m_camera.lock()->get_projection_matrix()));
	// if doing specular light update it to pass in the camera position in world space.

	glUniform3fv(u_light_direction_loc, 1, glm::value_ptr(m_directional_light_direction));
	glUniform3fv(u_light_colour_loc, 1, glm::value_ptr(m_directional_light_colour));
	glUniform3fv(u_ambient_light_colour_loc, 1, glm::value_ptr(m_ambient_light_colour));
}

void renderer::switch_to_3d_static_mesh_shader()
{
	using namespace gl;
	if (m_current_shader_program == m_static_geometry_program_id) return;
	glUseProgram(m_static_geometry_program_id);
	m_current_shader_program = m_static_geometry_program_id;
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glFrontFace(GL_CW);   // LH system requires clockwise winding, CW = Clock Wise winding order.

	// set the uniforms (renderer level). as they appear in: source/library/render/shaders/static_mesh_shader.h
	// other uniforms are to be set by the renderable object.
	const GLint u_view_loc = glGetUniformLocation(m_static_geometry_program_id, "u_view");
	const GLint u_projection_loc = glGetUniformLocation(m_static_geometry_program_id, "u_projection");
	const GLint u_light_direction_loc = glGetUniformLocation(m_static_geometry_program_id, "u_light_direction");
	const GLint u_light_colour_loc = glGetUniformLocation(m_static_geometry_program_id, "u_light_colour");
	const GLint u_ambient_light_colour_loc = glGetUniformLocation(m_static_geometry_program_id, "u_ambient_light_colour");

	glUniformMatrix4fv(u_view_loc, 1, GL_FALSE, glm::value_ptr(m_camera.lock()->get_view_matrix()));
	glUniformMatrix4fv(u_projection_loc, 1, GL_FALSE, glm::value_ptr(m_camera.lock()->get_projection_matrix()));
	// if doing specular light update it to pass in the camera position in world space.

	// just hard code the light: direction & color, also the ambient light. Add in the control later if needed.
	glUniform3fv(u_light_direction_loc, 1, glm::value_ptr(m_directional_light_direction));
	glUniform3fv(u_light_colour_loc, 1, glm::value_ptr(glm::vec3{ 1.0f, 1.0f, 1.0f }));
	glUniform3fv(u_ambient_light_colour_loc, 1, glm::value_ptr(glm::vec3{ 0.1f, 0.1f, 0.1f }));

	glUniform3fv(u_light_colour_loc, 1, glm::value_ptr(m_directional_light_colour));
	glUniform3fv(u_ambient_light_colour_loc, 1, glm::value_ptr(m_ambient_light_colour));
}

void renderer::switch_to_2d_shader()
{
	using namespace gl;
	if (m_current_shader_program == m_textured_quad_geometry_program_id) return;
	glUseProgram(m_textured_quad_geometry_program_id);
	m_current_shader_program = m_textured_quad_geometry_program_id;

	// set the uniforms (renderer level). as they appear in: source/library/render/shaders/textured_quad_shader.h
	// other uniforms are to be set by the renderable object.
	const GLint u_resolution_location = glGetUniformLocation(m_textured_quad_geometry_program_id, "u_resolution");
	if (u_resolution_location != -1)
	{
		glUniform2fv(u_resolution_location, 1, glm::value_ptr(m_framebuffer_size));
	}
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glCullFace(GL_NONE);
}
