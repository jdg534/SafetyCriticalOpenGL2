#ifndef _VERTEXT_TYPES_H_
#define _VERTEXT_TYPES_H_

#include <glm/glm.hpp>

namespace vertex_types
{

	struct vertex_2d
	{
		glm::vec2 position;
		glm::vec2 texture_coordinates;
	};

	static constexpr size_t vertex2d_struct_size = sizeof(vertex_2d);

	struct vertex_3d
	{
		glm::vec3 position;
		glm::vec2 texture_coordinates;
		glm::vec3 normal;
	};

	static constexpr size_t vertex3d_struct_size = sizeof(vertex_3d);
	
	// todo, if need vertex type for terrain.
	
	// rigged geomentry later if coming back to the project
}

#endif // _VERTEXT_TYPES_H_