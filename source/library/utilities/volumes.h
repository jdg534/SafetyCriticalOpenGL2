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
	};

	struct axis_aligned_bounding_cube : public axis_aligned_bounding_box
	{
		float min_z, max_z;
	};

	namespace checks
	{
		bool do_spheres_overlap(const bounding_sphere& a, const bounding_sphere& b);
		bool do_boxes_overlap_y_axis_up(const axis_aligned_bounding_box& a, const axis_aligned_bounding_box& b);
		bool do_cubes_overlap_left_hand(const axis_aligned_bounding_cube& a, const axis_aligned_bounding_cube& b);
	}
}

#endif // _VOLUMES_H_