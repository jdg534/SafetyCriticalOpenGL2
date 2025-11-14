#ifndef _LIBRARY_MAIN_H_
#define _LIBRARY_MAIN_H_

#include "render/include_opengl.h"
#include "render/renderer.h"
#include "assets/asset_manager.h"

#include <memory>

// temp includes. "should" move scene stuff into scene class.
// also a UI for the root or 2d scenes. note "should" not "must" for "current scope".

#include "render/2d/text/text_block.h"
#include "render/2d/quad.h"
#include "render/3d/static_model.h"


class library_main
{
public:

	library_main();
	~library_main();

	void run();

private:

	void initialise();
	GLFWwindow* initialise_window();
	void initialise_test_data();
	void shutdown();
	void shutdown_test_data();

	static void s_on_framebuffer_resize(GLFWwindow* window, int width, int height);
	void on_framebuffer_resize(GLFWwindow* window, int width, int height);
	void tick(float delta_time);

	GLFWwindow* m_window { nullptr };
	std::unique_ptr<renderer> m_renderer;
	std::shared_ptr<asset_manager> m_asset_manager; // needs to be constructed via make_shared, weak_ptr to be passed around

	// temp assets, for testing.
	std::shared_ptr<static_model> m_test_cube;
	std::shared_ptr<text_block> m_test_text;
	std::shared_ptr<quad> m_test_quad;
	std::shared_ptr<quad> m_red_test_quad;
	std::shared_ptr<quad> m_green_test_quad;
	std::shared_ptr<quad> m_blue_test_quad;
	std::shared_ptr<quad> m_magenta_test_quad;
	std::shared_ptr<quad> m_test_smiley_quad;


	static library_main* s_instance_ptr;
};

#endif // _LIBRARY_MAIN_H_
