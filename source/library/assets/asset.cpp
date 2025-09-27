#include "asset.h"

#include <string>
#include <string_view>

asset::asset(const std::string& name, std::weak_ptr<const asset_manager> asset_manager)
	: m_name(name)
	, m_asset_manager(asset_manager)
{}

asset::~asset()
{}

std::string_view asset::get_name() const
{
	return m_name;
}

std::weak_ptr<const asset_manager> asset::get_asset_manager() const
{
	return m_asset_manager;
}