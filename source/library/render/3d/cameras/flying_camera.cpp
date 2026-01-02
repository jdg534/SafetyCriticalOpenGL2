#include "flying_camera.h"

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.inl>
#include <algorithm>

// public
/////////

flying_camera::flying_camera(GLFWwindow* window)
	: camera()
	, m_window(window)
{
	m_move_forward_key = GLFW_KEY_W;
	m_move_back_key = GLFW_KEY_S;
	m_move_left_key = GLFW_KEY_A;
	m_move_right_key = GLFW_KEY_D;
	m_move_up_key = GLFW_KEY_Q;
	m_move_down_key = GLFW_KEY_E;
	m_turn_up_key = GLFW_KEY_UP;
	m_turn_down_key = GLFW_KEY_DOWN;
	m_turn_left_key = GLFW_KEY_LEFT;
	m_turn_right_key = GLFW_KEY_RIGHT;
	m_increase_move_speed_key = GLFW_KEY_PAGE_UP;
	m_decrease_move_speed_key = GLFW_KEY_PAGE_DOWN;

	set_position({0.0f,10.0f, 0.0f});
	set_look_at_position({0.0f, 0.0f, 10.0f});
}

flying_camera::~flying_camera()
{

}

void flying_camera::tick(float delta_time)
{
	handle_rotation(delta_time);
	handle_movement(delta_time);
	handle_speed_change(delta_time);
	set_look_at_position(get_position() + m_forward);
}

bool flying_camera::is_moving() const
{
	return m_is_moving;
}

float flying_camera::get_move_speed() const
{
	return m_move_speed;
}

// private
//////////

void flying_camera::handle_rotation(float delta_time)
{
	const float delta = m_rotation_speed * delta_time;

	if (glfwGetKey(m_window, m_turn_left_key) == GLFW_PRESS) m_yaw -= delta;
	if (glfwGetKey(m_window, m_turn_right_key) == GLFW_PRESS) m_yaw += delta;
	if (glfwGetKey(m_window, m_turn_up_key) == GLFW_PRESS) m_pitch += delta;
	if (glfwGetKey(m_window, m_turn_down_key) == GLFW_PRESS) m_pitch -= delta;

	constexpr float pitch_limit = glm::radians(89.0f);
	m_pitch = std::clamp(m_pitch, -pitch_limit, pitch_limit);

	m_forward.x = cosf(m_pitch) * sinf(m_yaw);
	m_forward.y = sinf(m_pitch);
	m_forward.z = cosf(m_pitch) * cosf(m_yaw);

	m_forward = glm::normalize(m_forward);
}

void flying_camera::handle_movement(float delta_time)
{
	const float velocity = m_move_speed * delta_time;

	const glm::vec3 forward = glm::normalize(get_look_at_position() - get_position());
	const glm::vec3 right = glm::normalize(glm::cross(get_up_vector(), forward));
	const glm::vec3 up = glm::normalize(get_up_vector());

	glm::vec3 movement{ 0.0f };

	if (glfwGetKey(m_window, m_move_forward_key) == GLFW_PRESS) movement += forward;
	if (glfwGetKey(m_window, m_move_back_key) == GLFW_PRESS) movement -= forward;
	if (glfwGetKey(m_window, m_move_right_key) == GLFW_PRESS) movement += right;
	if (glfwGetKey(m_window, m_move_left_key) == GLFW_PRESS) movement -= right;

	if (glfwGetKey(m_window, m_move_up_key) == GLFW_PRESS) movement += up;
	if (glfwGetKey(m_window, m_move_down_key) == GLFW_PRESS) movement -= up;

	m_is_moving = glm::length2(movement) > 0.0f;

	if (m_is_moving)
	{
		set_position(get_position() + glm::normalize(movement) * velocity);
	}
}

void flying_camera::handle_speed_change(float delta_time)
{
	if (glfwGetKey(m_window, m_increase_move_speed_key) == GLFW_PRESS) m_move_speed += delta_time * 10.0f;
	if (glfwGetKey(m_window, m_decrease_move_speed_key) == GLFW_PRESS) m_move_speed -= delta_time * 10.0f;
}
