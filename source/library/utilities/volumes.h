#ifndef _VOLUMES_H_
#define _VOLUMES_H_

#include <glm/glm.hpp>

namespace volumes
{
	struct bounding_sphere
	{
		float radius;
		glm::vec3 location;
	};

	struct axis_aligned_bounding_box
	{
		float min_x, max_x;
		float min_y, max_y;
		float min_z, max_z;
	};

	namespace checks
	{
		static bool do_spheres_overlap(const bounding_sphere& a, const bounding_sphere& b);
		static bool do_boxes_overlap_left_hand(const axis_aligned_bounding_box& a, const axis_aligned_bounding_box& b);
	}
}

#endif // _VOLUMES_H_