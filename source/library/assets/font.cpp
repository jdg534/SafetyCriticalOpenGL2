#include "font.h"

#include "../utilities/text_utilities.h"

#include <algorithm>
#include <fstream>
#include <string_view>
#include <sstream>

#include <rapidjson/document.h>
#include <rapidjson/encodings.h>

using namespace rapidjson;

// public
/////////

font::font(const std::string& name, std::weak_ptr<const asset_manager> asset_manager)
	: asset(name, asset_manager)
{

}

font::~font()
{

}

void font::initialise(std::string_view file_path)
{
	std::ifstream assets_list_file(file_path.data());
	if (!assets_list_file.good())
	{
		throw std::exception("asset_manager::initialise failed to open file");
	}
	std::stringstream buffer;
	buffer << assets_list_file.rdbuf();

	Document font_info;
	if (font_info.Parse(buffer.str().c_str()).HasParseError())
	{
		throw std::exception("asset_manager::initialise failed to parse the json");
	}
	assets_list_file.close();
	buffer.clear();

	m_atlas_asset_name = font_info["atlas_asset_name"].GetString();
	m_default_spacing = font_info["default_spacing"].GetFloat();
	initialise_glyph_info(font_info);
	initialise_kerning_info(font_info);
}

void font::shutdown()
{
	m_kerning_info.clear();
	m_glyph_info.clear();
}

asset_type font::get_type() const
{
	return asset_type::font;
}

bool font::is_string_supported(const std::vector<char32_t>& to_check) const
{
	// checks all characters are supported by the font.
	using namespace std;
	return all_of(begin(to_check), end(to_check),
		[this](char32_t character)
		{
			if (text_utilities::is_character_white_space(character))
			{
				return true;
			}
			return any_of(begin(m_glyph_info), end(m_glyph_info),
				[character](const glyph_info& supported_glyph)
				{
					return supported_glyph.glyph == character;
				});
		});
}

// private
//////////

void font::initialise_glyph_info(const Document& font_file)
{
	const auto& glyph_entries = font_file["glyphs"].GetArray();
	const int num_glyphs = glyph_entries.Size();
	m_glyph_info.resize(num_glyphs);
	for (int i = 0; i < num_glyphs; ++i)
	{
		const auto& glyph_info = glyph_entries[i].GetObject();
		m_glyph_info[i].glyph = text_utilities::utf8_to_char32(glyph_info["character"].GetString());
		const auto& source_rectangle = glyph_info["source_rectangle"].GetObject();
		m_glyph_info[i].top_px = source_rectangle["top"].GetFloat();
		m_glyph_info[i].bottom_px = m_glyph_info[i].top_px + source_rectangle["height"].GetFloat();
		m_glyph_info[i].left_px = source_rectangle["left"].GetFloat();
		m_glyph_info[i].right_px = m_glyph_info[i].left_px + source_rectangle["width"].GetFloat();
	}
}

void font::initialise_kerning_info(const Document& font_file)
{
	const auto& kerning_entries = font_file["kerning"];
	const int num_kerning_entries = kerning_entries.Size();
	m_kerning_info.resize(num_kerning_entries);
	for (int i = 0; i < num_kerning_entries; ++i)
	{
		const auto& kerning_info = kerning_entries[i].GetObject();
		m_kerning_info[i].additional_spacing = kerning_info["addition_spacing"].GetFloat();
		m_kerning_info[i].previous_glyph = text_utilities::utf8_to_char32(kerning_info["previous_glyph"].GetString());
		m_kerning_info[i].current_glyph = text_utilities::utf8_to_char32(kerning_info["current_glyph"].GetString());
	}
}
