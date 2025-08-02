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

	asset* load_asset(const std::string& name, const std::string& type, const std::string& path);

	std::vector<asset*> m_assets;
};
