#include "asset_manager.h"

#include <exception>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <sstream>

#include <rapidjson/document.h>

#include "asset_utils.h"
#include "../render/include_opengl.h"
#include "font.h"
#include "texture.h"
#include "3d/model.h"

void asset_manager::initialise(std::string_view assets_list_file_path)
{
	std::ifstream assets_list_file(assets_list_file_path.data());
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
	const std::string_view assets_relative_to = asset_utils::get_directory_path(assets_list_file_path);
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
		const std::string actual_file_path = std::string(assets_relative_to) + assets_entries[i]["path"].GetString();
		m_assets[i] = load_asset(assets_entries[i]["name"].GetString(),
			assets_entries[i]["type"].GetString(),
			actual_file_path);
	}
}

void asset_manager::shutdown()
{
	for (auto asset : m_assets)
	{
		asset->shutdown();
	}
	m_assets.clear();
}

std::weak_ptr<asset> asset_manager::get_asset_on_name(std::string_view asset_name) const
{
	for (const auto asset : m_assets)
	{
		if (asset->get_name() == asset_name)
		{
			return asset;
		}
	}
	throw std::runtime_error(std::string("Could not find the asset with name: ") + asset_name.data());
}

std::weak_ptr<asset> asset_manager::get_asset_on_path(std::string_view asset_path) const
{
	for (const auto asset : m_assets)
	{
		if (asset->get_path() == asset_path)
		{
			return asset;
		}
	}
	throw std::runtime_error(std::string("Could not find the asset with path: ") + asset_path.data());
}

void asset_manager::request_load_texture(std::string_view name, std::string_view file_path)
{
	throw std::runtime_error("CODE asset_manager::request_load_texture");
}

asset_type asset_manager::to_type(std::string_view s)
{
	if (s == "texture") return asset_type::texture;
	if (s == "font") return asset_type::font;
	if (s == "model") return asset_type::model;
	return asset_type::invalid;
}

std::shared_ptr<asset> asset_manager::load_asset(std::string_view name, std::string_view type, std::string_view path)
{
	switch (to_type(type))
	{
		case asset_type::texture: return load_texture(name, path);
		case asset_type::font: return load_font(name, path);
		case asset_type::model: return load_model(name, path);
		default: throw std::exception("asset type not recognised"); break;
	}
	return nullptr;
}

// TODO: break this up into asset_loader. which knows about the manager.
std::shared_ptr<asset> asset_manager::load_texture(std::string_view name, std::string_view path)
{
	std::shared_ptr<texture> result = std::make_shared<texture>(name.data(), path.data(), weak_from_this());
	result->initialise();
	return result;
}

std::shared_ptr<asset> asset_manager::load_font(std::string_view name, std::string_view path)
{
	std::shared_ptr<font> result = std::make_shared<font>(name.data(), path.data(), weak_from_this());
	result->initialise();
	return result;
}

std::shared_ptr<asset> asset_manager::load_model(std::string_view name, std::string_view path)
{
	std::shared_ptr<model> result = std::make_shared<model>(name.data(), path.data(), weak_from_this());
	result->initialise();
	return result;
}
