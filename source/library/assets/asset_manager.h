#pragma once

#include <string>
#include "asset.h"

#include <vector>

class asset_manager
{
public:

	void initialise(std::string_view assets_list_file_path);
	void shutdown();

private:

	static asset_type to_type(std::string_view s);

	asset* load_asset(std::string_view name, std::string_view type, std::string_view path);
	asset* load_texture(std::string_view name, std::string_view path);
	asset* load_font (std::string_view name, std::string_view path);
	asset* load_static_model (std::string_view name, std::string_view path);
	asset* load_rigged_model (std::string_view name, std::string_view path);
	asset* load_materials(std::string_view name, std::string_view path);

	std::vector<asset*> m_assets; // make these shared_ptr<asset>. refactor!
};
