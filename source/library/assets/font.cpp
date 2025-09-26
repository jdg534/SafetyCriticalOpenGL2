#include "font.h"

// public
/////////


font::font(const std::string& name, std::weak_ptr<asset_manager> asset_manager)
	: asset(name, asset_manager)
{

}

font::~font()
{

}

void font::initialise(std::string_view file_path)
{
	// todo: code it.
}

void font::shutdown()
{

}

asset_type font::get_type() const
{
	return asset_type::font;
}

// private
//////////
