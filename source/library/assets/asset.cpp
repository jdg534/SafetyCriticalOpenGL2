#include "asset.h"

#include <string>
#include <string_view>

// public
/////////

asset::asset(const std::string& name, const std::string& path, std::weak_ptr<const asset_manager> asset_manager)
	: m_name(name)
	, m_path(path)
	, m_asset_manager(asset_manager)
{}

asset::~asset() {}
std::string_view asset::get_name() const { return m_name; }
std::string_view asset::get_path() const { return m_path; }

// protected
////////////

std::weak_ptr<const asset_manager> asset::get_asset_manager() const
{
	return m_asset_manager;
}