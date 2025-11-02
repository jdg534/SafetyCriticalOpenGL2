#include "model.h"

#include <string_view>

model::model(const std::string& name, std::weak_ptr<const asset_manager> asset_manager)
	: asset(name, asset_manager)
{

}

model::~model()
{
	// TODO: CODE ME!
}

void model::initialise(std::string_view file_path)
{
	// TODO: CODE ME!
}

void model::shutdown()
{
	// TODO: CODE ME!
}

asset_type model::get_type() const
{
	return asset_type::model;
}