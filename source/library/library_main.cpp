#include "library_main.h"

#include "render/renderer.h"
#include "render/include_opengl.h"

#include "assets/font.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <exception>
#include <string>

#include <rapidjson/document.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.inl>

// temp test drawable values.

library_main* library_main::s_instance_ptr = nullptr;

// public
/////////

library_main::library_main()
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
	m_renderer = std::make_unique<renderer>(glm::vec2(flt_framebuffer_width, flt_framebuffer_height), 50);
	m_renderer->initialise();
	glfwSetFramebufferSizeCallback(m_window, library_main::s_on_framebuffer_resize);

	m_asset_manager = std::make_shared<asset_manager>();
	m_asset_manager->initialise("assets/assets_list.json");

	std::weak_ptr<asset> font_asset_ptr = m_asset_manager->get_asset_on_name("font");

	std::weak_ptr<font> font_ptr = std::dynamic_pointer_cast<font>(font_asset_ptr.lock());
	std::vector<char32_t> text_to_display;
	// some text.
	text_to_display.resize(11);
	text_to_display[0] = 's'; text_to_display[1] = 'o'; text_to_display[2] = 'm'; text_to_display[3] = 'e';
	text_to_display[4] = ' ';
	text_to_display[5] = 't'; text_to_display[6] = 'e'; text_to_display[7] = 'x'; text_to_display[8] = 't';
	text_to_display[9] = '.'; text_to_display[10] = 0; // null terminator character.


	// todo: any setup for objects that are to be used defining stuff to render.
	m_test_text = std::make_shared<text_block>(text_to_display, font_ptr, 50);
	m_test_text->initialise();
	
	const glm::vec2 test_quad_size{ 100.0f, 100.0f };
	std::weak_ptr<texture> test_quad_texture = std::dynamic_pointer_cast<texture>(m_asset_manager->get_asset_on_name("plain_white").lock());
	m_test_quad = std::make_shared<quad>(test_quad_texture, test_quad_size);
	m_test_quad->initialise();
	m_test_quad->set_tint({1.0f, 1.0f, 0.0f, 1.0f});
	m_red_test_quad = std::make_shared<quad>(test_quad_texture, test_quad_size);
	m_red_test_quad->initialise();
	m_red_test_quad->set_tint({1.0f, 0.0f, 0.0f, 1.0f});
	m_green_test_quad = std::make_shared<quad>(test_quad_texture, test_quad_size);
	m_green_test_quad->initialise();
	m_green_test_quad->set_tint({ 0.0f, 1.0f, 0.0f, 1.0f });
	m_blue_test_quad = std::make_shared<quad>(test_quad_texture, test_quad_size);
	m_blue_test_quad->initialise();
	m_blue_test_quad->set_tint({ 0.0f, 0.0f, 1.0f, 1.0f });
	m_magenta_test_quad = std::make_shared<quad>(test_quad_texture, test_quad_size);
	m_magenta_test_quad->initialise();
	m_magenta_test_quad->set_tint({ 1.0f, 0.0f, 1.0f, 1.0f });

	m_renderer->add_to_render_list(m_test_quad);
	m_renderer->add_to_render_list(m_red_test_quad);
	m_renderer->add_to_render_list(m_green_test_quad);
	m_renderer->add_to_render_list(m_blue_test_quad);
	m_renderer->add_to_render_list(m_magenta_test_quad);

	m_renderer->add_to_render_list(m_test_text); // remember the painters algorithm! want the text on top.
	m_renderer->sort_render_list();


	/*
	after this line add flag to not permit allocations. those aren't permitted after initialisation. also free() / delete.
	only during initialisation should allocations be tolerated.
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

void library_main::shutdown()
{
	for (auto test_quad : {m_test_quad,m_red_test_quad, m_green_test_quad, m_blue_test_quad, m_magenta_test_quad })
	{
		test_quad->shutdown();
		test_quad.reset();
	}
	if (m_test_text)
	{
		m_test_text->shutdown();
		m_test_text.reset();
	}
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

void library_main::s_on_framebuffer_resize(GLFWwindow* window, int width, int height)
{
	s_instance_ptr->on_framebuffer_resize(window, width, height);
}

void library_main::on_framebuffer_resize(GLFWwindow* window, int width, int height)
{
	m_renderer->set_framebuffer_size(glm::vec2(static_cast<float>(width), static_cast<float>(height)));
}

void library_main::tick(float delta_time)
{
	using namespace glm;

	static constexpr float delta_time_cap = 0.25f;
	const float delta_time_to_use = std::min(delta_time, delta_time_cap);
	static float angle = 0.0f, turn_speed_degrees = 1.0f;
	angle += turn_speed_degrees * delta_time_to_use;

	int frame_buffer_width = 0, frame_buffer_height = 0;
	glfwGetFramebufferSize(m_window, &frame_buffer_width, &frame_buffer_height);
	const float flt_fbw = static_cast<float>(frame_buffer_width), flt_fbh = static_cast<float>(frame_buffer_height);
	
	mat4x4 translate_matrix = identity<mat4x4>();
	vec3 translate_to_middle_of_screen = { flt_fbw * 0.5f, frame_buffer_height * 0.5f, 0.0f };
	translate_matrix = translate(translate_matrix, translate_to_middle_of_screen);
	mat4x4 rotate_matrix = identity<mat4x4>();
	rotate_matrix = rotate(rotate_matrix, angle, { 0.0f, 0.0f, 1.0f });
	mat4x4 transform = identity<mat4x4>();
	transform = translate_matrix * rotate_matrix;
	m_test_quad->set_transform(transform);

	m_red_test_quad->set_transform(translate(identity<mat4x4>(), { 0.0f, 0.0f, 0.0f })); // should be top left
	m_green_test_quad->set_transform(translate(identity<mat4x4>(), {flt_fbw, 0.0f, 0.0f})); // should be top right
	m_blue_test_quad->set_transform(translate(identity<mat4x4>(), { flt_fbw, flt_fbh, 0.0f }));  // should be bottom right
	m_magenta_test_quad->set_transform(translate(identity<mat4x4>(), { 0.0f, flt_fbh, 0.0f }));  // should be bottom left

	m_test_text->set_transform(translate(identity<mat4x4>(), {200.0f, 200.0f, 0.0f}));
}
