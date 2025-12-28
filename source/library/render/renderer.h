#ifndef _RENDERER_H_
#define _RENDERER_H_

#include "include_opengl.h"

#include <glbinding/gl/types.h>
#include <glm/glm.hpp>

#include <memory>
#include <vector>

#include "renderable.decl.h"
#include "3d/camera.decl.h"

class renderer
{
public:

	renderer() = delete;
	explicit renderer(glm::vec2 framebuffer_size, const size_t render_list_cap, std::weak_ptr<const camera> camera);

	void initialise();
	void shutdown();

	void render_frame();

	void add_to_render_list(std::weak_ptr<renderable> to_add);
	// todo(if needed): remove_from_render_list()
	void sort_render_list();

	void set_framebuffer_size(glm::vec2 framebuffer_size);
	void set_camera(std::weak_ptr<const camera> camera);

	glm::vec4 get_clear_colour() const;
	glm::vec3 get_ambient_light_colour() const;
	glm::vec3 get_directional_light_colour() const;
	glm::vec3 get_directional_light_direction() const;

	void get_clear_colour(glm::vec4 colour);
	void get_ambient_light_colour(glm::vec3 colour);
	void get_directional_light_colour(glm::vec3 colour);
	void get_directional_light_direction(glm::vec3 direction);


private:

	void initialise_shaders();

	void shutdown_shaders();

	void switch_to_terrain_shader();
	void switch_to_3d_static_mesh_shader();
	void switch_to_2d_shader();

	gl::GLuint m_static_geometry_vertex_shader_object_id = 0,
		m_static_geometry_fragment_shader_id = 0,
		m_static_geometry_program_id = 0;
	gl::GLuint m_textured_quad_geometry_vertex_shader_object_id = 0,
		m_textured_quad_geometry_fragment_shander_id = 0,
		m_textured_quad_geometry_program_id = 0;
	gl::GLuint m_terrain_vertex_shader_object_id = 0,
		m_terrain_fragment_shander_id = 0,
		m_terrain_program_id = 0;

	gl::GLuint m_current_shader_program = 0;

	glm::vec2 m_framebuffer_size;
	glm::vec4 m_clear_colour { 0.5f,0.5f,0.5f, 1.0f };

	glm::vec3 m_ambient_light_colour{ 0.2f, 0.2f, 0.2f };
	glm::vec3 m_directional_light_colour{ 0.8f, 0.8f, 0.8f };
	glm::vec3 m_directional_light_direction{ 0.0f, -1.0f, 0.0f };

	std::vector<std::weak_ptr<renderable>> m_render_list;
	std::weak_ptr<const camera> m_camera;
	const size_t m_render_list_cap { 0 };
};

#endif // _RENDERER_H_
