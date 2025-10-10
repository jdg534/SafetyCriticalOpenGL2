#pragma once

#include "render/include_opengl.h"
#include "render/renderer.h"
#include "assets/asset_manager.h"

#include <memory>
// temp includes. "should" move scene stuff into scene class.

class library_main
{
public:

	void run();

private:

	void initialise();
	GLFWwindow* initialise_window();
	void shutdown();

	GLFWwindow* m_window { nullptr };
	std::unique_ptr<renderer> m_renderer;
	std::unique_ptr < asset_manager> m_asset_manager;
};

