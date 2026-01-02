#ifndef _FLYING_CAMERA_H_
#define _FLYING_CAMERA_H_

#include "../camera.h"
#include "../../../tickable.h"

#include "../../include_opengl.h"

class flying_camera
	: public camera
	, public i_tickable
{
public:

	explicit flying_camera(GLFWwindow* window);
	virtual ~flying_camera();

	void tick(float delta_time) override;

	bool is_moving() const;

private:

	void handle_rotation(float delta_time);
	void handle_movement(float delta_time);
	void handle_speed_change(float delta_time);

	bool m_is_moving = false;

	// LLM generated code.
	float m_move_speed = 5.0f;
	float m_rotation_speed = 1.5f;
	float m_yaw = 0.0f;
	float m_pitch = 0.0f;
	glm::vec3 m_forward{ 0.0f, 0.0f, 1.0f };

	// glfw API friendly types used.
	int m_move_forward_key = 0;
	int m_move_back_key = 0;
	int m_move_left_key = 0;
	int m_move_right_key = 0;
	int m_move_up_key = 0;
	int m_move_down_key = 0;
	int m_turn_up_key = 0;
	int m_turn_down_key = 0;
	int m_turn_left_key = 0;
	int m_turn_right_key = 0;
	int m_increase_move_speed_key = 0;
	int m_decrease_move_speed_key = 0;
	GLFWwindow* m_window = nullptr; // note this doesn't own the window!
};


#endif // _FLYING_CAMERA_H_