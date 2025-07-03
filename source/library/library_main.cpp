#include "library_main.h"

#include "render/renderer.h"
#include "render/include_opengl.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <exception>
#include <string>

#include <rapidjson/document.h>

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
	m_renderer = new renderer(m_window);
	m_renderer->initialise();
}

GLFWwindow* library_main::initialise_window()
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // todo figure out correct values of SC 2.0
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // ''
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // ''

	int window_width = 800;
	int window_height = 600;
	std::string window_title = "A window";

	// refactor to std::filesystem::exists("assets/window_settings.json") force C++17
	std::ifstream window_settings_file("assets/window_settings.json");
	if (window_settings_file.good())
	{
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
	if (m_renderer)
	{
		m_renderer->shutdown();
		delete m_renderer;
		m_renderer = nullptr;
	}
	if (m_window)
	{
		glfwDestroyWindow(m_window);
		m_window = nullptr;
	}
	glfwTerminate();
}
