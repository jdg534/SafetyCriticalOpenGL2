#pragma once

#include <cstdint>

enum class asset_type : std::uint8_t
{
	invalid,
	texture,
	font,
	static_model, // out of scope for branch
	rigged_model, // out of scope for branch, treated as a should have. depending on what assimp outputs, we could merge this with static_model
	materials // out of scope for branch
};