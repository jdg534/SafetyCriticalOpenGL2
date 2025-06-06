#pragma once
// HI C++ might not accept this.if so go to #ifndef #define #endif

#include <GLFW/glfw3.h>

// class for 
class library_main
{
public:

	void run();

private:

	void initialise();
	void shutdown();

	GLFWwindow* m_window { nullptr };
};

