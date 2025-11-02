#ifndef _ASSET_TYPES_H_
#define _ASSET_TYPES_H_

#include <cstdint>

enum class asset_type : std::uint8_t
{
	invalid,
	texture,
	font,
	model,
	mesh, // loaded when loading a model
	material, // loaded when loading a model
};

#endif // _ASSET_TYPES_H_
