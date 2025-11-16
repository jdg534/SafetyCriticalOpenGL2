#include "camera.h"

#include <glm/gtc/matrix_transform.hpp>

camera::camera()
{

}

camera::~camera()
{

}

glm::mat4x4 camera::get_view_matrix() const
{
	return glm::lookAtLH(get_position(), get_look_at_position(), get_up_vector());
}

glm::mat4x4 camera::get_projection_matrix() const
{
	const float aspect_ratio = get_view_port_width() / get_view_port_height();
	return glm::perspectiveLH_NO(get_field_of_view_angle_radians(), aspect_ratio, get_near_clipping_distance(), get_far_clipping_distance());
}

const glm::vec3& camera::get_position() const { return m_position; }
void camera::set_position(const glm::vec3& position) { m_position = position; }
const glm::vec3& camera::get_world_position() const { return get_position(); } // override if doing a mounted camera.
const glm::vec3& camera::get_look_at_position() const { return m_look_at_position; }
void camera::set_look_at_position(const glm::vec3& look_at_position) { m_look_at_position = look_at_position; }
void camera::set_look_at_position_from_direction(const glm::vec3& look_at_direction) { m_look_at_position = get_position() + look_at_direction; }
const glm::vec3& camera::get_up_vector() const { return m_up_vector; }
void camera::set_up_vector(const glm::vec3& up_vector) { m_up_vector = up_vector; }
float camera::get_field_of_view_angle_radians() const { return m_field_of_view_angle_radians; }
void camera::set_field_of_view_angle_radians(float angle) { m_field_of_view_angle_radians = angle; }
float camera::get_view_port_width() const { return m_view_port_width; }
void camera::set_view_port_width(float width) { m_view_port_width = width; }
float camera::get_view_port_height() const { return m_view_port_height; }
void camera::set_view_port_height(float height) { m_view_port_height = height; }
float camera::get_near_clipping_distance() const { return m_near_clipping_distance; }
void camera::set_near_clipping_distance(float distance) { m_near_clipping_distance = distance; }
float camera::get_far_clipping_distance() const { return m_far_clipping_distance; }
void camera::set_far_clipping_distance(float distance) { m_far_clipping_distance = distance; }
