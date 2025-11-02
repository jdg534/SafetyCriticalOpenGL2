#ifndef _ASSET_TYPES_H_
#define _ASSET_TYPES_H_

#include <cstdint>

enum class asset_type : std::uint8_t
{
	invalid,
	texture,
	font,
	model, // only foucus on static_models, see if can incorporate material loading.
};

#endif // _ASSET_TYPES_H_
