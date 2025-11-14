#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <glm/glm.hpp>

class camera
{

public:

	camera();
	virtual ~camera();

	virtual glm::mat4x4 get_view_matrix() const;
	glm::mat4x4 get_projection_matrix() const;

	const glm::vec3& get_position() const;
	void set_position(const glm::vec3& position);
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

private:

	glm::vec3 m_position;
	glm::vec3 m_look_at_position;
	glm::vec3 m_up_vector;
	float m_field_of_view_angle_radians = { 0.785398f }; // 45 degrees
	float m_view_port_width = { 800.0f };
	float m_view_port_height ={ 600.0f };
	float m_near_clipping_distance = { 0.001f };
	float m_far_clipping_distance = { 1000.0f };
};

#endif // _CAMERA_H_