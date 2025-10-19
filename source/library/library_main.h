#pragma once

#include "render/include_opengl.h"
#include "render/renderer.h"
#include "assets/asset_manager.h"

#include <memory>

// temp includes. "should" move scene stuff into scene class.
// also a UI for the root or 2d scenes. note "should" not "must" for "current scope".

#include "render/2d/text/text_block.h"

class library_main
{
public:

	library_main();
	~library_main();

	void run();

private:

	void initialise();
	GLFWwindow* initialise_window();
	void shutdown();

	static void s_on_framebuffer_resize(GLFWwindow* window, int width, int height);
	void on_framebuffer_resize(GLFWwindow* window, int width, int height);

	GLFWwindow* m_window { nullptr };
	std::unique_ptr<renderer> m_renderer;
	std::shared_ptr<asset_manager> m_asset_manager; // needs to be constructed via make_shared, weak_ptr to be passed around

	// temp text assets
	std::shared_ptr<text_block> m_test_text;

	static library_main* s_instance_ptr;
};

