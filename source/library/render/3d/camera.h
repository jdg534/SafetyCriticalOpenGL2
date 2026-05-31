#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <glm/glm.hpp>
#include <array>

struct frustum
{
	std::array<glm::vec4,6> planes;
};

class camera
{

public:

	camera() = default;
	camera(const camera& other) = delete;
	camera(camera&& to_move) = delete;
	virtual ~camera() = default;

	camera& operator=(const camera&) = delete;
	camera& operator=(camera&&) = delete;

	virtual glm::mat4x4 get_view_matrix() const;
	glm::mat4x4 get_projection_matrix() const;

	const glm::vec3& get_position() const;
	void set_position(const glm::vec3& position);
	virtual const glm::vec3& get_world_position() const;
	const glm::vec3& get_look_at_position() const;
	void set_look_at_position(const glm::vec3& look_at_position);
	void set_look_at_position_from_direction(const glm::vec3& look_at_direction);
	const glm::vec3& get_up_vector() const;
	void set_up_vector(const glm::vec3& up_vector);

	float get_field_of_view_angle_radians() const;
	void set_field_of_view_angle_radians(float angle);

	float get_view_port_width() const;
	void set_view_port_width(float width);

	float get_view_port_height() const;
	void set_view_port_height(float height);

	float get_near_clipping_distance() const;
	void set_near_clipping_distance(float distance);

	float get_far_clipping_distance() const;
	void set_far_clipping_distance(float distance);

	frustum get_frustrum() const;

private:

	glm::vec3 m_position = { 0.0F, 0.0F, 0.0F };
	glm::vec3 m_look_at_position = { 0.0F, 0.0F, 10.0F};
	glm::vec3 m_up_vector = {0.0F, 1.0F, 0.0F};
	float m_field_of_view_angle_radians = { 0.785398F }; // 45 degrees
	float m_view_port_width = { 800.0F };
	float m_view_port_height ={ 600.0F };
	float m_near_clipping_distance = { 0.001F };
	float m_far_clipping_distance = { 800.0F * 1000.0F }; // GPT says max view distence is 800km for a fast jet, using that since viewing from space would be overkill.
};

#endif // _CAMERA_H_