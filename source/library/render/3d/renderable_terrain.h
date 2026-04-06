#ifndef _RENDERABLE_TERRAIN_H_
#define _RENDERABLE_TERRAIN_H_

#include <memory>

#include "renderable_3d.h"
#include "camera.h"
#include "../../assets/3d/terrain/terrain.h"

#include "../../utilities/volumes.h"

// note that this doesn't own any data.

class renderable_terrain : public renderable_3d
{

public:

	renderable_terrain() = delete;
	renderable_terrain(std::weak_ptr<const terrain> terrain);
	virtual ~renderable_terrain();

	void initialise() override;
	void shutdown() override;
	void draw() override;

	void set_active_camera(std::weak_ptr<const camera> active_camera);

	float get_clip_area_meters_padding() const;
	void set_clip_area_meters_padding(float padding_in_meters);

private:

	volumes::axis_aligned_bounding_box get_aabb_of_camera_view_area() const;
	static volumes::axis_aligned_bounding_box tile_area_to_aabb(const renderable_tile_area& area);

	std::weak_ptr<const terrain> m_terrain;
	std::weak_ptr<const camera> m_active_camera;

	float m_clip_area_meters_padding = 2000.0f;
};

#endif // _RENDERABLE_TERRAIN_H_
