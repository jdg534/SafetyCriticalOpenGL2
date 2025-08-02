#include "asset_manager.h"

#include <exception>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <rapidjson/document.h>

void asset_manager::initialise(const std::string& assets_list_file_path)
{
	std::ifstream assets_list_file(assets_list_file_path);
	if (!assets_list_file.good())
	{
		throw std::exception("asset_manager::initialise failed to open file");
	}
	std::stringstream buffer;
	buffer << assets_list_file.rdbuf();
	rapidjson::Document doc;
	if (doc.Parse(buffer.str().c_str()).HasParseError())
	{
		throw std::exception("asset_manager::initialise failed to parse the json");
	}
	assets_list_file.close();
	buffer.clear();
	if (!doc.HasMember("assets"))
	{
		throw std::exception("asset_manager::initialise now assets");
	}
	const auto assets_entries = doc["assets"].GetArray();
	const int num_assets = assets_entries.Size();
	m_assets.resize(num_assets);
	for (int i = 0; i < num_assets; ++i)
	{
		//"name": "string"
		//"type" : "string"
		//"path" : "string"

		if (!assets_entries[i].HasMember("name")
		|| !assets_entries[i].HasMember("type")
		|| !assets_entries[i].HasMember("path"))
		{
			throw std::exception("asset_manager::initialise an asset doesn't have a required field");
		}
		m_assets[i] = load_asset(assets_entries[i]["name"].GetString(),
			assets_entries[i]["type"].GetString(),
			assets_entries[i]["path"].GetString());
	}
}

void asset_manager::shutdown()
{
	for (auto asset : m_assets)
	{
		asset->shutdown();
		delete asset;
	}
	m_assets.clear();
}

asset* asset_manager::load_asset(const std::string& name, const std::string& type, const std::string& path)
{
	throw "Code ME!";
	return nullptr;
}
