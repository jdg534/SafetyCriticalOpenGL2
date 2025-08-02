#pragma once

#include "render/include_opengl.h"
#include "render/renderer.h"
#include "assets/asset_manager.h"

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
	asset_manager* m_asset_manager { nullptr };
};

