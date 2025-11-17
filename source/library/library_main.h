#ifndef _LIBRARY_MAIN_H_
#define _LIBRARY_MAIN_H_

#include "render/include_opengl.h"
#include "render/renderer.h"
#include "assets/asset_manager.h"

#include <memory>
#include <glm/fwd.hpp>

// temp includes. "should" move scene stuff into scene class.
// also a UI for the root or 2d scenes. note "should" not "must" for "current scope".

#include "render/2d/text/text_block.h"
#include "render/2d/quad.h"
#include "render/3d/static_model.h"
#include "render/3d/camera.h"


class library_main
{
public:

	library_main();
	~library_main();

	void run();

private:

	void initialise();
	GLFWwindow* initialise_window();
	void initialise_test_data();
	void shutdown();
	void shutdown_test_data();

	static void s_on_framebuffer_resize(GLFWwindow* window, int width, int height);
	void on_framebuffer_resize(GLFWwindow* window, int width, int height);
	static void s_on_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void on_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

	void on_left_pressed();
	void on_right_pressed();
	void on_up_pressed();
	void on_down_pressed();
	void on_page_up_pressed();
	void on_page_down_pressed();
	void on_left_released();
	void on_right_released();
	void on_up_released();
	void on_down_released();
	void on_page_up_released();
	void on_page_down_released();

	void on_w_pressed();
	void on_a_pressed();
	void on_s_pressed();
	void on_d_pressed();
	void on_q_pressed();
	void on_e_pressed();
	void on_r_pressed();
	void on_w_released();
	void on_a_released();
	void on_s_released();
	void on_d_released();
	void on_q_released();
	void on_e_released();

	void tick(float delta_time);

	void update_text_tints();
	static void update_text_tint(std::shared_ptr<text_block> to_update,bool use_change_colour);

	GLFWwindow* m_window { nullptr };
	std::unique_ptr<renderer> m_renderer;
	std::shared_ptr<asset_manager> m_asset_manager; // needs to be constructed via make_shared, weak_ptr to be passed around

	// temp assets, for testing.
	std::shared_ptr<static_model> m_test_cube;
	std::shared_ptr<camera> m_camera;
	std::shared_ptr<text_block> m_cube_position_text;
	std::shared_ptr<text_block> m_camera_position_text;
	std::shared_ptr<text_block> m_camera_look_at_position_text;
	std::shared_ptr<quad> m_textured_quad;

	glm::vec3 m_camera_movement_speed;
	glm::vec3 m_camera_look_at_point_movement_speed;

	static library_main* s_instance_ptr;
};

#endif // _LIBRARY_MAIN_H_
