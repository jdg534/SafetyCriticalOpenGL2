#pragma once

#include <string>
#include "asset.h"

#include <vector>

class asset_manager
{
public:

	void initialise(const std::string& assets_list_file_path);
	void shutdown();

private:

	static asset_type to_type(const std::string& s);

	asset* load_asset(const std::string& name, const std::string& type, const std::string& path);
	asset* load_texture(const std::string& name, const std::string& path);
	asset* load_font (const std::string& name, const std::string& path);
	asset* load_static_model (const std::string& name, const std::string& path);
	asset* load_rigged_model (const std::string& name, const std::string& path);
	asset* load_materials(const std::string& name, const std::string& path);

	std::vector<asset*> m_assets; // make these shared_ptr<asset>. refactor!
};
