#ifndef _VERTEXT_TYPES_H_
#define _VERTEXT_TYPES_H_

#include <glm/glm.hpp>

struct vertex_2d
{
	glm::vec2 position;
	glm::vec2 texture_coordinates;
	glm::vec3 blend_colour;
};

struct vertex_3d
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texture_coordinates;
	glm::vec3 blend_colour;
};

// todo, if needed vertex type of terrain. rigged geomentry later if coming back to the project

#endif // _VERTEXT_TYPES_H_