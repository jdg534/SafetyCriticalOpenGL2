#include "asset_manager.h"

#include <exception>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <sstream>

#include <rapidjson/document.h>

#include <png.h>

#include "asset_utils.h"
#include "../render/include_opengl.h"
#include "font.h"
#include "texture.h"

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
	// windows style file paths aren't considered.
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

asset_type asset_manager::to_type(std::string_view s)
{
	if (s == "texture") return asset_type::texture;
	if (s == "font") return asset_type::font;
	if (s == "static_model") return asset_type::static_model;
	if (s == "rigged_model") return asset_type::rigged_model;
	if (s == "materials") return asset_type::materials;
	return asset_type::invalid;
}

std::shared_ptr<asset> asset_manager::load_asset(std::string_view name, std::string_view type, std::string_view path)
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

// break this up into asset_loader.
std::shared_ptr<asset> asset_manager::load_texture(std::string_view name, std::string_view path)
{
	// into image::initialise()
	if (path.find(".png") == std::string::npos)
	{
		throw std::exception("load texture called with non png texture.");
	}
	FILE* file = fopen(path.data(), "rb"); // path needs to be relevent to the assets list.
	if (!file)
	{
		throw std::exception("failed to load png file");
	}
	// Read header
	png_byte header[8];
	fread(header, 1, 8, file);
	if (png_sig_cmp(header, 0, 8)) {
		fclose(file);
		throw std::runtime_error(std::string("File is not a PNG: ") + path.data());
	}
	// Init png structures
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (!png_ptr) {
		fclose(file);
		throw std::runtime_error("png_create_read_struct failed");
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_read_struct(&png_ptr, nullptr, nullptr);
		fclose(file);
		throw std::runtime_error("png_create_info_struct failed");
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
		fclose(file);
		throw std::runtime_error("Error during PNG read");
	}

	png_init_io(png_ptr, file);
	png_set_sig_bytes(png_ptr, 8);
	png_read_info(png_ptr, info_ptr);

	const unsigned width = png_get_image_width(png_ptr, info_ptr);
	const unsigned height = png_get_image_height(png_ptr, info_ptr);
	const png_byte color_type = png_get_color_type(png_ptr, info_ptr);
	const png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

	// Convert palettes/gray/16-bit to RGBA8
	if (bit_depth == 16) { png_set_strip_16(png_ptr); }
	if (color_type == PNG_COLOR_TYPE_PALETTE) { png_set_palette_to_rgb(png_ptr); }
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) { png_set_expand_gray_1_2_4_to_8(png_ptr); }
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) { png_set_tRNS_to_alpha(png_ptr); }
	if (color_type == PNG_COLOR_TYPE_RGB ||
		color_type == PNG_COLOR_TYPE_GRAY ||
		color_type == PNG_COLOR_TYPE_PALETTE)
	{ png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER); }

	if (color_type == PNG_COLOR_TYPE_GRAY ||
		color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
	{ png_set_gray_to_rgb(png_ptr); }

	png_read_update_info(png_ptr, info_ptr);

	// Allocate pixel buffer
	std::vector<unsigned char> pixels(width * height * 4);
	std::vector<png_bytep> row_pointers(height);
	for (unsigned y = 0; y < height; y++) {
		row_pointers[y] = pixels.data() + y * width * 4;
	}

	png_read_image(png_ptr, row_pointers.data());

	// Cleanup
	png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
	fclose(file);

	// gl code.
	gl::GLuint texture_id;
	gl::glGenTextures(1, &texture_id);
	gl::glBindTexture(gl::GL_TEXTURE_2D, texture_id);

	// Upload pixel data
	gl::glTexImage2D(
		gl::GL_TEXTURE_2D, 0, gl::GL_RGBA,
		width, height, 0,
		gl::GL_RGBA, gl::GL_UNSIGNED_BYTE,
		pixels.data()
	);

	// Generate mipmaps, SC ok?
	gl::glGenerateMipmap(gl::GL_TEXTURE_2D);

	// Texture parameters
	gl::glTexParameteri(gl::GL_TEXTURE_2D, gl::GL_TEXTURE_WRAP_S, gl::GL_REPEAT);
	gl::glTexParameteri(gl::GL_TEXTURE_2D, gl::GL_TEXTURE_WRAP_T, gl::GL_REPEAT);
	gl::glTexParameteri(gl::GL_TEXTURE_2D, gl::GL_TEXTURE_MIN_FILTER, gl::GL_LINEAR_MIPMAP_LINEAR);
	gl::glTexParameteri(gl::GL_TEXTURE_2D, gl::GL_TEXTURE_MAG_FILTER, gl::GL_LINEAR);
	
	return std::make_shared<texture>(name.data(), shared_from_this(), texture_id, width, height);
}

std::shared_ptr<asset> asset_manager::load_font(std::string_view name, std::string_view path)
{
	std::shared_ptr<font> result = std::make_shared<font>(name.data(), shared_from_this());
	result->initialise(path);
	return result;
}

std::shared_ptr<asset> asset_manager::load_static_model(std::string_view name, std::string_view path)
{
	throw std::exception(__func__);
}

std::shared_ptr<asset> asset_manager::load_rigged_model(std::string_view name, std::string_view path)
{
	throw std::exception(__func__);
}

std::shared_ptr<asset> asset_manager::load_materials(std::string_view name, std::string_view path)
{
	throw std::exception(__func__);
}
