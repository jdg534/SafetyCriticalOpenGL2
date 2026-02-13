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

	float get_height_at(float x_world_space, float z_world_space) const;
	void set_active_camera(std::weak_ptr<const camera> active_camera);

private:

	static volumes::axis_aligned_bounding_box tile_area_to_aabb(const renderable_tile_area& area);
	static bool is_renderable_tile_area_in_frustrum(const renderable_tile_area& area, const frustum& frustrum);

	std::weak_ptr<const terrain> m_terrain;
	std::weak_ptr<const camera> m_active_camera;
};

#endif // _RENDERABLE_TERRAIN_H_
