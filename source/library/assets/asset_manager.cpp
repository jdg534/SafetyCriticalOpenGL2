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

asset_type asset_manager::to_type(const std::string& s)
{
	if (s == "texture") return asset_type::texture;
	if (s == "font") return asset_type::font;
	if (s == "static_model") return asset_type::static_model;
	if (s == "rigged_model") return asset_type::rigged_model;
	if (s == "materials") return asset_type::materials;
	return asset_type::invalid;
}

asset* asset_manager::load_asset(const std::string& name, const std::string& type, const std::string& path)
{
	switch (to_type(type))
	{
		case asset_type::texture: return load_texture(name, path);
		case asset_type::font: return load_font(name, path);
		case asset_type::static_model: return load_static_model(name, path);
		case asset_type::rigged_model: return load_rigged_model(name, path);
		case asset_type::materials: return load_materials(name, path);
		default: throw std::exception("asset type not recognised"); break;
	}
	return nullptr;
}

asset* asset_manager::load_texture(const std::string& name, const std::string& path)
{
	throw std::exception(__func__);
}

asset* asset_manager::load_font(const std::string& name, const std::string& path)
{
	throw std::exception(__func__);
}

asset* asset_manager::load_static_model(const std::string& name, const std::string& path)
{
	throw std::exception(__func__);
}

asset* asset_manager::load_rigged_model(const std::string& name, const std::string& path)
{
	throw std::exception(__func__);
}

asset* asset_manager::load_materials(const std::string& name, const std::string& path)
{
	throw std::exception(__func__);
}
