#include "library_main.h"

#include "assets/font.h"
#include "assets/3d/model.h"
#include "render/3d/camera.h"
#include "render/include_opengl.h"
#include "render/renderer.h"
#include "utilities/text_utilities.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <exception>
#include <string>

#include <rapidjson/document.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.inl>

using namespace std;

library_main* library_main::s_instance_ptr = nullptr;

constexpr glm::vec4 text_changing_tint = { 1.0f, 0.0f, 0.0f, 1.0f };
constexpr glm::vec4 text_normal_tint   = { 1.0f, 1.0f, 1.0f, 1.0f };

// public
/////////

library_main::library_main()
	: m_camera_movement_speed(0.0f,0.0f,0.0f)
	, m_camera_look_at_point_movement_speed(0.0f, 0.0f, 0.0f)
{
	s_instance_ptr = this;
}

library_main::~library_main()
{
	s_instance_ptr = nullptr;
}

void library_main::run()
{
	initialise();
	static float running_time = 0.0f;
	while (!glfwWindowShouldClose(m_window))
	{
		const float delta_time = static_cast<float>(glfwGetTime()) - running_time;
		running_time += delta_time;
		tick(delta_time);
		m_renderer->render_frame();
		glfwSwapBuffers(m_window);
		glfwPollEvents();
	}
	shutdown();
}

// private
//////////

void library_main::initialise()
{
	if (glfwInit() != GLFW_TRUE)
	{
		throw std::exception("glfwInit() failed");
	}
	m_window = initialise_window();
	glfwMakeContextCurrent(m_window);
	glbinding::initialize(glfwGetProcAddress);
	int framebuffer_width = 0, framebuffer_height = 0;
	glfwGetFramebufferSize(m_window, &framebuffer_width, &framebuffer_height);
	const float flt_framebuffer_width = static_cast<float>(framebuffer_width), flt_framebuffer_height = static_cast<float>(framebuffer_height);

	m_camera = make_shared<camera>();
	m_camera->set_view_port_width(flt_framebuffer_width);
	m_camera->set_view_port_height(flt_framebuffer_height);
	m_camera->set_position({ 5.0f, 1.0f, -10.0f });

	m_renderer = make_unique<renderer>(glm::vec2(flt_framebuffer_width, flt_framebuffer_height), 50, m_camera);
	m_renderer->initialise();

	glfwSetFramebufferSizeCallback(m_window, library_main::s_on_framebuffer_resize);
	glfwSetKeyCallback(m_window, library_main::s_on_key_callback);

	m_asset_manager = std::make_shared<asset_manager>();
	m_asset_manager->initialise("assets/assets_list.json");

	initialise_test_data();

	for (auto text_block : { m_cube_position_text, m_camera_position_text, m_camera_look_at_position_text })
	{
		m_renderer->add_to_render_list(text_block);
	}
	m_renderer->add_to_render_list(m_textured_quad);
	// remember the painters algorithm for 2d stuff!

	// 3d stuff will be z buffered. (order doesn't matter).
	m_renderer->add_to_render_list(m_test_cube);
	m_renderer->add_to_render_list(m_terrain);
	m_renderer->sort_render_list();


	/*
	after this line add flag to not permit allocations to deallocations.
	allocations and deallocation should only be tolerated in initialisation and shutdown.

	A custom heap is to be addressed in a different pull request once terrain rendering is in.
	*/
}

GLFWwindow* library_main::initialise_window()
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // todo figure out correct values of SC 2.0
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // ''
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // ''
#ifdef NDEBUG
	// non debug code.
#else
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

	// GLFW_ANGLE_PLATFORM_TYPE_OPENGLES, look into this.

	int window_width = 800;
	int window_height = 600;
	std::string window_title = "A window";

	if (std::filesystem::exists("assets/window_settings.json"))
	{
		std::ifstream window_settings_file("assets/window_settings.json");
		std::stringstream buffer;
		buffer << window_settings_file.rdbuf();
		std::string json_str = buffer.str();

		rapidjson::Document doc;
		if (doc.Parse(json_str.c_str()).HasParseError()) {
			std::cerr << "JSON parse error\n";
			return nullptr;
		}
		window_settings_file.close();
		buffer.clear();
		json_str.clear();

		if (doc.HasMember("title") && doc["title"].IsString())
		{
			window_title = doc["title"].GetString();
		}
		if (doc.HasMember("width") && doc["width"].IsInt())
		{
			window_width = doc["width"].GetInt();
		}
		if (doc.HasMember("height") && doc["height"].IsInt())
		{
			window_height = doc["height"].GetInt();
		}
	}
	
	GLFWwindow* results = glfwCreateWindow(window_width, window_height, window_title.c_str(), nullptr, nullptr);
	if (results == nullptr)
	{
		const char* error_str[256];
		error_str[0] = '\0';
		const int error_code = glfwGetError(error_str);
		std::cerr << "glfwCreateWindow() failed: " << *error_str << std::endl;
		throw std::exception("glfwCreateWindow() failed");
	}
	return results;
}

void library_main::initialise_test_data()
{
	using namespace glm;
	weak_ptr<const asset> font_asset_ptr = m_asset_manager->get_asset_on_name("font");

	m_test_cube = make_shared<static_model>(dynamic_pointer_cast<const model>(m_asset_manager->get_asset_on_name("grass_cube").lock()));
	weak_ptr<const font> font_ptr = dynamic_pointer_cast<const font>(font_asset_ptr.lock());

	// setup for objects that are to be used for texting the rendered. Remember 2d stuff uses the painters algorithm.
	m_cube_position_text = make_shared<text_block>(U"Cube position: X.XXXX, Y.YYYY, Z.ZZZZ", font_ptr, 64, line_spaceing::RELATIVE_1_2);
	m_cube_position_text->initialise();
	m_camera_position_text = make_shared<text_block>(U"Camera position: : X.XXXX, Y.YYYY, Z.ZZZZ", font_ptr, 64, line_spaceing::RELATIVE_1_2);
	m_camera_position_text->initialise();
	m_camera_look_at_position_text = make_shared<text_block>(U"Camera look at position: : X.XXXX, Y.YYYY, Z.ZZZZ", font_ptr, 64, line_spaceing::RELATIVE_1_2);
	m_camera_look_at_position_text->initialise();

	m_camera_position_text->set_parent(m_cube_position_text);
	m_camera_look_at_position_text->set_parent(m_camera_position_text);
	for (auto camera_text_block : { m_camera_look_at_position_text , m_camera_position_text })
	{
		camera_text_block->set_transform(glm::translate(identity<mat4x4>(), { 0.0f, 25.0f, 0.0f }));
	}

	weak_ptr<const texture> smiley_texture = dynamic_pointer_cast<const texture>(m_asset_manager->get_asset_on_name("smiley").lock());
	m_textured_quad = make_shared<quad>(smiley_texture, vec2{50.0f, 50.0f});
	m_textured_quad->initialise();
	m_textured_quad->set_transform(glm::translate(identity<mat4x4>(), {45.0f, 175.0f, 0.0f}));

	weak_ptr<const terrain> test_terrain = dynamic_pointer_cast<const terrain>(m_asset_manager->get_asset_on_name("umbra_mount").lock());
	m_terrain = make_shared<renderable_terrain>(test_terrain);
	m_terrain->initialise();
}

void library_main::shutdown()
{
	shutdown_test_data();
	if (m_asset_manager)
	{
		m_asset_manager->shutdown();
		m_asset_manager.reset();
	}
	if (m_renderer)
	{
		m_renderer->shutdown();
		m_renderer.reset();
	}
	if (m_window)
	{
		glfwDestroyWindow(m_window);
		m_window = nullptr;
	}
	glfwTerminate();
}

void library_main::shutdown_test_data()
{
	for (auto text_block : { m_cube_position_text, m_camera_position_text, m_camera_look_at_position_text})
	{
		text_block->shutdown();
		text_block.reset();
	}
	m_textured_quad->shutdown();
	m_textured_quad.reset();
	m_test_cube->shutdown();
	m_test_cube.reset();
	m_terrain->shutdown();
	m_terrain.reset();
}

void library_main::s_on_framebuffer_resize(GLFWwindow* window, int width, int height)
{
	s_instance_ptr->on_framebuffer_resize(window, width, height);
}

void library_main::on_framebuffer_resize(GLFWwindow* window, int width, int height)
{
	const float flt_width = static_cast<float>(width);
	const float flt_height = static_cast<float>(height);
	m_renderer->set_framebuffer_size(glm::vec2(flt_width, flt_height));
	m_camera->set_view_port_width(flt_width);
	m_camera->set_view_port_height(flt_height);
}

void library_main::s_on_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	s_instance_ptr->on_key_callback(window, key, scancode, action, mods);
}

void library_main::on_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	const bool is_press_event = action == GLFW_PRESS;
	const bool is_release_event = action == GLFW_RELEASE;
	if (is_press_event == false
	&& is_release_event == false)
	{
		return;
	}

	if (is_press_event)
	{
		switch (key)
		{
			case GLFW_KEY_LEFT:      on_left_pressed();      break;
			case GLFW_KEY_RIGHT:     on_right_pressed();     break;
			case GLFW_KEY_UP:        on_up_pressed();        break;
			case GLFW_KEY_DOWN:      on_down_pressed();      break;
			case GLFW_KEY_PAGE_UP:   on_page_up_pressed();   break;
			case GLFW_KEY_PAGE_DOWN: on_page_down_pressed(); break;
			case GLFW_KEY_W:         on_w_pressed();         break;
			case GLFW_KEY_A:         on_a_pressed();         break;
			case GLFW_KEY_S:         on_s_pressed();         break;
			case GLFW_KEY_D:         on_d_pressed();         break;
			case GLFW_KEY_Q:         on_q_pressed();         break;
			case GLFW_KEY_E:         on_e_pressed();         break;
			case GLFW_KEY_R:         on_r_pressed();         break;
			default: break;
		}
	}
	else if (is_release_event)
	{
		switch (key)
		{
			case GLFW_KEY_LEFT:      on_left_released();      break;
			case GLFW_KEY_RIGHT:     on_right_released();     break;
			case GLFW_KEY_UP:        on_up_released();        break;
			case GLFW_KEY_DOWN:      on_down_released();      break;
			case GLFW_KEY_PAGE_UP:   on_page_up_released();   break;
			case GLFW_KEY_PAGE_DOWN: on_page_down_released(); break;
			case GLFW_KEY_W:         on_w_released();         break;
			case GLFW_KEY_A:         on_a_released();         break;
			case GLFW_KEY_S:         on_s_released();         break;
			case GLFW_KEY_D:         on_d_released();         break;
			case GLFW_KEY_Q:         on_q_released();         break;
			case GLFW_KEY_E:         on_e_released();         break;
			default: break;
		}
		
	}
	update_text_tints();
}

void library_main::on_left_pressed() { m_camera_look_at_point_movement_speed.x = -1.0f; }
void library_main::on_right_pressed() { m_camera_look_at_point_movement_speed.x = 1.0f; }
void library_main::on_up_pressed() { m_camera_look_at_point_movement_speed.z = 1.0f; }
void library_main::on_down_pressed() { m_camera_look_at_point_movement_speed.z = -1.0f; }
void library_main::on_page_up_pressed() { m_camera_look_at_point_movement_speed.y = 1.0f; }
void library_main::on_page_down_pressed() { m_camera_look_at_point_movement_speed.y = -1.0f; }
void library_main::on_left_released() { m_camera_look_at_point_movement_speed.x = 0.0f; }
void library_main::on_right_released() { m_camera_look_at_point_movement_speed.x = 0.0f; }
void library_main::on_up_released() { m_camera_look_at_point_movement_speed.z = 0.0f; }
void library_main::on_down_released() { m_camera_look_at_point_movement_speed.z = 0.0f; }
void library_main::on_page_up_released() { m_camera_look_at_point_movement_speed.y = 0.0f; }
void library_main::on_page_down_released() { m_camera_look_at_point_movement_speed.y = 0.0f; }
void library_main::on_w_pressed() { m_camera_movement_speed.z = 1.0f; }
void library_main::on_a_pressed() { m_camera_movement_speed.x = -1.0f; }
void library_main::on_s_pressed() { m_camera_movement_speed.z = -1.0f; }
void library_main::on_d_pressed() { m_camera_movement_speed.x = 1.0f; }
void library_main::on_q_pressed() { m_camera_movement_speed.y = 1.0f; }
void library_main::on_e_pressed() { m_camera_movement_speed.y = -1.0f; }

void library_main::on_r_pressed()
{
	m_camera_movement_speed = m_camera_look_at_point_movement_speed = glm::vec3(0.0f, 0.0f, 0.0f);
	m_camera->set_look_at_position({ 0.0f, 0.0f, 0.0f });
	m_camera->set_position({ 5.0f, 1.0f, -10.0f });
}

void library_main::on_w_released() { m_camera_movement_speed.z = 0.0f; }
void library_main::on_a_released() { m_camera_movement_speed.x = 0.0f; }
void library_main::on_s_released() { m_camera_movement_speed.z = 0.0f; }
void library_main::on_d_released() { m_camera_movement_speed.x = 0.0f; }
void library_main::on_q_released() { m_camera_movement_speed.y = 0.0f; }
void library_main::on_e_released() { m_camera_movement_speed.y = 0.0f; }

void library_main::tick(float delta_time)
{
	using namespace glm;
	using namespace std;

	static constexpr float delta_time_cap = 0.25f;
	const float delta_time_to_use = std::min(delta_time, delta_time_cap);
	static float angle = 0.0f, turn_speed_degrees = 1.0f;
	angle += turn_speed_degrees * delta_time_to_use;

	int frame_buffer_width = 0, frame_buffer_height = 0;
	glfwGetFramebufferSize(m_window, &frame_buffer_width, &frame_buffer_height);
	const float flt_fbw = static_cast<float>(frame_buffer_width), flt_fbh = static_cast<float>(frame_buffer_height);
	m_test_cube->set_transform(rotate(identity<mat4x4>(), angle, { 0.0f, 1.0f, 0.0f }));

	vec3 camera_position = m_camera->get_position();
	vec3 camera_look_at_position = m_camera->get_look_at_position();
	camera_position += m_camera_movement_speed * delta_time_to_use;
	camera_look_at_position += m_camera_look_at_point_movement_speed * delta_time_to_use;
	m_camera->set_position(camera_position);
	m_camera->set_look_at_position(camera_look_at_position);

	// this is post initialisation, we'll want a different heap that has fixed size.
	const vec3 cube_position = m_test_cube->get_net_transform()[3];
	u32string text_to_set = U"Cube position: ";
	text_utilities::append_vec3(text_to_set, cube_position);
	m_cube_position_text->set_text(text_to_set);
	text_to_set = U"Camera position: ";
	text_utilities::append_vec3(text_to_set, camera_position);
	m_camera_position_text->set_text(text_to_set);
	text_to_set = U"Camera look at position: ";
	text_utilities::append_vec3(text_to_set, camera_look_at_position);
	m_camera_look_at_position_text->set_text(text_to_set);
}

void library_main::update_text_tints()
{
	update_text_tint(m_camera_position_text, length(m_camera_movement_speed) > 0.0f);
	update_text_tint(m_camera_look_at_position_text, length(m_camera_look_at_point_movement_speed) > 0.0f);
}

void library_main::update_text_tint(std::shared_ptr<text_block> to_update, bool use_change_colour)
{
	to_update->set_tint(use_change_colour ? text_changing_tint : text_normal_tint);
}
