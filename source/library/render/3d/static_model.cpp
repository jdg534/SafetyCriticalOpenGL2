#include "static_model.h"

#include "../../assets/3d/model.h"

// public
/////////

static_model::static_model(std::weak_ptr<model> model)
	: renderable_3d()
	, m_model(model)
{
	set_renderable_type(renderable_type::STATIC_GEOMETRY);
}

static_model::~static_model()
{
	renderable_3d::~renderable_3d();
}

void static_model::initialise()
{
	// place holder.
}

void static_model::shutdown()
{
	// nothing to shutdown, this doesn't own the data.
}

void static_model::draw()
{
	/* uniforms to set:
	uniform mat4 u_model;
	uniform mat4 u_view; // TODO, refactor to be u_view_projection matrix. once once got it working.
	uniform mat4 u_projection; // See above. move it to the renderer (only changes once pre frame.)
	uniform sampler2D u_diffuse_map;
	
	uniform vec3      u_camera_position;     // in world-space
	uniform vec3      u_light_direction;     // unit vectors only
	uniform vec3      u_light_colour;
	uniform vec3      u_ambient_light_colour; // light colour
	uniform vec4      u_surface_tint;        // colour multiplier
	*/
}