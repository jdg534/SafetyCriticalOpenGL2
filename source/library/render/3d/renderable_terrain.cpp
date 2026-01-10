#include "renderable_terrain.h"

#include <glm/gtc/type_ptr.inl>

#include "../../assets/3d/material.h"
#include "../../assets/3d/mesh.h"
#include "../../assets/3d/model.h"

#include "../../assets/texture.h"

// public
/////////

renderable_terrain::renderable_terrain(std::weak_ptr<const terrain> terrain)
	: renderable_3d()
	, m_terrain(terrain)
{
	set_renderable_type(renderable_type::TERRAIN);
}

renderable_terrain::~renderable_terrain()
{
	renderable_3d::~renderable_3d();
}

void renderable_terrain::initialise()
{
	// place holder.
}

void renderable_terrain::shutdown()
{
	// nothing to shutdown, this doesn't own the data.
}

void renderable_terrain::draw()
{
	using namespace gl;

	const GLuint shader_id = get_shader_program();
	const GLint u_model_location = glGetUniformLocation(shader_id, "u_model");
	const GLint u_splat_map_location = glGetUniformLocation(shader_id, "u_splat_map");
	const GLint u_red_channel_diffuse_map_location = glGetUniformLocation(shader_id, "u_red_channel_diffuse_map");
	const GLint u_green_channel_diffuse_map_location = glGetUniformLocation(shader_id, "u_green_channel_diffuse_map");
	const GLint u_blue_channel_diffuse_map_location = glGetUniformLocation(shader_id, "u_blue_channel_diffuse_map");
	const GLint u_alpha_channel_diffuse_map_location = glGetUniformLocation(shader_id, "u_alpha_channel_diffuse_map");


	const glm::mat4x4 net_transform = get_net_transform();

	glUniformMatrix4fv(u_model_location, 1, GL_FALSE, glm::value_ptr(net_transform));

	auto terrain = m_terrain.lock();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, terrain->get_splat_map_texture_id());
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, terrain->get_red_channel_mapped_texture_texture_id());
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, terrain->get_green_channel_mapped_texture_texture_id());
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, terrain->get_blue_channel_mapped_texture_texture_id());
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, terrain->get_alpha_channel_mapped_texture_texture_id());

	glUniform1i(u_splat_map_location, 0); // 0 as in GL_TEXTURE0
	glUniform1i(u_red_channel_diffuse_map_location, 1);
	glUniform1i(u_green_channel_diffuse_map_location, 2);
	glUniform1i(u_blue_channel_diffuse_map_location, 3);
	glUniform1i(u_alpha_channel_diffuse_map_location, 4);




	// buffers
	for (const auto& terrain_cell : terrain->get_renderable_tiles())
	{
		// if it's in the camera's FOV draw it. but now draw all.



		glBindVertexArray(terrain_cell.vertex_array_object_id);
		glBindBuffer(GL_ARRAY_BUFFER, terrain_cell.vertex_buffer_id);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrain_cell.index_buffer_id);

		// draw
		glDrawElements(GL_TRIANGLES, terrain_cell.num_indices_to_draw, GL_UNSIGNED_INT, 0);
	}

	// unbind the textures
	for (auto texture_slot_index : { GL_TEXTURE0,GL_TEXTURE1,GL_TEXTURE2,GL_TEXTURE3,GL_TEXTURE4 })
	{
		glActiveTexture(texture_slot_index);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glBindVertexArray(0); // clear the vertex array.
}

float renderable_terrain::get_height_at(float x_world_space, float z_world_space) const
{
	return m_terrain.lock()->get_height_at(x_world_space, z_world_space);
}

void renderable_terrain::set_active_camera(std::weak_ptr<const camera> active_camera)
{
	m_active_camera = active_camera;
}
