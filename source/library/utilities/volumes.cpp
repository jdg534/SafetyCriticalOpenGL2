#include "volumes.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

bool volumes::checks::do_spheres_overlap(const bounding_sphere& a, const bounding_sphere& b)
{
	const float net_squared_radii = (a.radius * a.radius) + (b.radius * b.radius);
	return net_squared_radii >= glm::distance2(a.location, b.location);
}

bool volumes::checks::do_boxes_overlap_left_hand(const axis_aligned_bounding_box& a, const axis_aligned_bounding_box& b)
{
	if (a.min_x > b.max_x) return false;
	if (a.max_x < b.min_x) return false;
	if (a.min_y > b.max_y) return false;
	if (a.max_y < b.min_y) return false;
	if (a.min_z > b.max_z) return false;
	if (a.max_z < b.min_z) return false;
	return true;
}
