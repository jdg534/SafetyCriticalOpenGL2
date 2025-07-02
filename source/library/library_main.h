#pragma once

struct GLFWwindow;

#include "render/renderer.h"

// class for 
class library_main
{
public:

	void run();

private:

	void initialise();

	void shutdown();

	GLFWwindow* m_window { nullptr };
	renderer* m_renderer { nullptr };
};

