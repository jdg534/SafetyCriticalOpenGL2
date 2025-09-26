#pragma once

#include <string>
#include "asset.h"

#include <vector>
#include <memory>

class asset_manager : public std::enable_shared_from_this<asset_manager>
{
public:

	void initialise(std::string_view assets_list_file_path);
	void shutdown();

	std::weak_ptr<asset> get_asset_on_name(std::string_view asset_name) const;

private:

	static asset_type to_type(std::string_view s);

	std::shared_ptr<asset> load_asset(std::string_view name, std::string_view type, std::string_view path);
	std::shared_ptr<asset> load_texture(std::string_view name, std::string_view path);
	std::shared_ptr<asset> load_font(std::string_view name, std::string_view path);
	std::shared_ptr<asset> load_static_model(std::string_view name, std::string_view path);
	std::shared_ptr<asset> load_rigged_model(std::string_view name, std::string_view path);
	std::shared_ptr<asset> load_materials(std::string_view name, std::string_view path);

	std::vector<std::shared_ptr<asset>> m_assets;
};
