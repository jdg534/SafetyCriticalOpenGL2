#ifndef _RENDERABLE_TERRAIN_H_
#define _RENDERABLE_TERRAIN_H_

#include <memory>

#include "renderable_3d.h"
#include "../../assets/3d/terrain/terrain.h"

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

private:

	std::weak_ptr<const terrain> m_terrain;
};

#endif // _RENDERABLE_TERRAIN_H_
