#pragma once

#include "render/include_opengl.h"
#include "render/renderer.h"

class library_main
{
public:

	void run();

private:

	void initialise();
	GLFWwindow* initialise_window();
	void shutdown();

	GLFWwindow* m_window { nullptr };
	renderer* m_renderer { nullptr };
};

