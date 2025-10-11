#include "library_main.h"

#include "render/renderer.h"
#include "render/include_opengl.h"

#include "assets/font.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <exception>
#include <string>

#include <rapidjson/document.h>
#include <filesystem>

// temp test drawable values.

// public
/////////

void library_main::run()
{
	initialise();
	static float running_time = 0.0f;
	while (!glfwWindowShouldClose(m_window))
	{
		const float delta_time = static_cast<float>(glfwGetTime()) - running_time;
		running_time += delta_time;
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
	m_renderer = std::make_unique<renderer>(m_window);
	m_renderer->initialise();

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
	m_test_text = std::make_unique<text_block>(text_to_display, font_ptr, 50);
	m_test_text->initialise();

	// todo: any exposing stuff to the renderer (draw lists)

	// after this line add flag to not permit allocations. those aren't permitted after initialisation. also free() / delete.
}

GLFWwindow* library_main::initialise_window()
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // todo figure out correct values of SC 2.0
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // ''
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // ''
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
