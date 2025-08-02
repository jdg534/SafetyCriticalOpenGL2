#pragma once

#include <cstdint>

enum class asset_type : std::uint8_t
{
	invalid,
	texture,
	font,
	model, // out of scope for branch
	material // out of scope for branch
};