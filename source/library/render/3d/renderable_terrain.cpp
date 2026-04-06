#include "renderable_terrain.h"

#include <glm/gtc/type_ptr.inl>
#include <algorithm>

#include "../../assets/3d/material.h"
#include "../../assets/3d/mesh.h"
#include "../../assets/3d/model.h"
#include "../../assets/texture.h"
#include "../../utilities/volumes.h"

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
	using namespace volumes;

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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, terrain->get_green_channel_mapped_texture_texture_id());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, terrain->get_blue_channel_mapped_texture_texture_id());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, terrain->get_alpha_channel_mapped_texture_texture_id());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glUniform1i(u_splat_map_location, 0); // 0 as in GL_TEXTURE0
	glUniform1i(u_red_channel_diffuse_map_location, 1);
	glUniform1i(u_green_channel_diffuse_map_location, 2);
	glUniform1i(u_blue_channel_diffuse_map_location, 3);
	glUniform1i(u_alpha_channel_diffuse_map_location, 4);


	assert(!m_active_camera.expired());
	const axis_aligned_bounding_box view_area = get_aabb_of_camera_view_area();

	// buffers
	for (const auto& terrain_cell : terrain->get_renderable_tiles())
	{
		// if (checks::do_boxes_overlap_y_axis_up(view_area, tile_area_to_aabb(terrain_cell))) // bring it back later.
		{
			glBindVertexArray(terrain_cell.vertex_array_object_id);
			glBindBuffer(GL_ARRAY_BUFFER, terrain_cell.vertex_buffer_id);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrain_cell.index_buffer_id);

			// draw
			glDrawElements(GL_TRIANGLES, terrain_cell.num_indices_to_draw, GL_UNSIGNED_SHORT, 0);
		}
	}

	// unbind the textures
	for (auto texture_slot_index : { GL_TEXTURE0,GL_TEXTURE1,GL_TEXTURE2,GL_TEXTURE3,GL_TEXTURE4 })
	{
		glActiveTexture(texture_slot_index);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glBindVertexArray(0); // clear the vertex array.
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void renderable_terrain::set_active_camera(std::weak_ptr<const camera> active_camera)
{
	m_active_camera = active_camera;
}

float renderable_terrain::get_clip_area_meters_padding() const
{
	return m_clip_area_meters_padding;
}

void renderable_terrain::set_clip_area_meters_padding(float padding_in_meters)
{
	m_clip_area_meters_padding = padding_in_meters;
}

// private
//////////

volumes::axis_aligned_bounding_box renderable_terrain::get_aabb_of_camera_view_area() const
{
	using namespace glm;
	assert(!m_active_camera.expired());

	auto camera = m_active_camera.lock();

	const vec3& camera_position = camera->get_position();
	const vec3& camera_at_position = camera->get_look_at_position();
	const vec3 forward_3d = normalize(camera_at_position - camera_position);
	vec2 forward_2d = normalize(vec2(forward_3d.x, forward_3d.z)); // x is same, z is changed to y.


	const float half_fov_radians = camera->get_field_of_view_angle_radians() * 0.5f;
	const float max_distance = camera->get_far_clipping_distance();

	// Rotate forward vector left and right
	const float cos_fov = cos(half_fov_radians);
	const float sin_fov = sin(half_fov_radians);

	vec2 pos_2d = vec2(camera_position.x, camera_position.z);

	vec2 left_dir(forward_2d.x * cos_fov - forward_2d.y * sin_fov,
		forward_2d.x * sin_fov + forward_2d.y * cos_fov);

	vec2 right_dir(forward_2d.x * cos_fov + forward_2d.y * sin_fov,
		-forward_2d.x * sin_fov + forward_2d.y * cos_fov);

	// Compute far corners
	vec2 far_center = pos_2d + forward_2d * max_distance;
	vec2 far_left = pos_2d + left_dir * max_distance;
	vec2 far_right = pos_2d + right_dir * max_distance;

	volumes::axis_aligned_bounding_box result;
	result.min_x = std::min({ pos_2d.x, far_left.x, far_right.x, far_center.x });
	result.max_x = std::max({ pos_2d.x, far_left.x, far_right.x, far_center.x });
	result.min_y = std::min({ pos_2d.y, far_left.y, far_right.y, far_center.y });
	result.max_y = std::max({ pos_2d.y, far_left.y, far_right.y, far_center.y });

	// Optional safety margin to reduce pop-in
	const float margin = get_clip_area_meters_padding();
	result.min_x -= margin;
	result.max_x += margin;
	result.min_y -= margin;
	result.max_y += margin;

	return result;
}

volumes::axis_aligned_bounding_box renderable_terrain::tile_area_to_aabb(const renderable_tile_area& area)
{
	volumes::axis_aligned_bounding_box result;
	result.min_x = area.west_edge_in_meters;
	result.max_x = area.east_edge_in_meters;
	result.min_y = area.south_edge_in_meters;
	result.max_y = area.north_edge_in_meters;
	return result;
}
