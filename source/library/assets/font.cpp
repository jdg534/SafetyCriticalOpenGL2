#include "font.h"

#include <fstream>
#include <string_view>
#include <sstream>

#include <rapidjson/document.h>
#include <rapidjson/encodings.h>


// public
/////////

char32_t utf8_to_char32(const char* utf8str)
{
	// llm generated... only 1 character length strings should be used.
	const unsigned char* s = reinterpret_cast<const unsigned char*>(utf8str);
	if (*s < 0x80)
	{
		return *s;
	}
	else if ((*s >> 5) == 0x6)
	{
		return ((s[0] & 0x1F) << 6) | (s[1] & 0x3F);
	}
	else if ((*s >> 4) == 0xE)
	{
		return ((s[0] & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
	}
	else if ((*s >> 3) == 0x1E)
	{
		return ((s[0] & 0x07) << 18) | ((s[1] & 0x3F) << 12)
			|  ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
	}
	throw std::runtime_error("Invalid UTF-8 sequence");
}

font::font(const std::string& name, std::weak_ptr<asset_manager> asset_manager)
	: asset(name, asset_manager)
{

}

font::~font()
{

}

void font::initialise(std::string_view file_path)
{
	using namespace rapidjson;
	std::ifstream assets_list_file(file_path.data());
	if (!assets_list_file.good())
	{
		throw std::exception("asset_manager::initialise failed to open file");
	}
	std::stringstream buffer;
	buffer << assets_list_file.rdbuf();

	Document doc;
	if (doc.Parse(buffer.str().c_str()).HasParseError())
	{
		throw std::exception("asset_manager::initialise failed to parse the json");
	}
	assets_list_file.close();
	buffer.clear();

	m_atlas_asset_name = doc["atlas_asset_name"].GetString();
	m_default_spacing = doc["default_spacing"].GetFloat();

	// <read_glyphs()>
	const auto& glyph_entries = doc["glyphs"].GetArray();
	const int num_glyphs = glyph_entries.Size();
	m_glyph_in_image_values.resize(num_glyphs);
	for (int i = 0; i < num_glyphs; ++i)
	{
		const auto& glyph_info = glyph_entries[i].GetObject();
		m_glyph_in_image_values[i].glyph = utf8_to_char32(glyph_info["character"].GetString());
		const auto& source_rectangle = glyph_info["source_rectangle"].GetObject();
		m_glyph_in_image_values[i].top_px = source_rectangle["top"].GetFloat();
		m_glyph_in_image_values[i].bottom_px = m_glyph_in_image_values[i].top_px + source_rectangle["height"].GetFloat();
		m_glyph_in_image_values[i].left_px = source_rectangle["left"].GetFloat();
		m_glyph_in_image_values[i].right_px = m_glyph_in_image_values[i].left_px + source_rectangle["width"].GetFloat();
	}
	// </read_glyphs()>
	// <read_kerning()>
	const auto& kerning_entries = doc["kerning"];
	const int num_kerning_entries = kerning_entries.Size();
	m_kerning_info.resize(num_kerning_entries);
	for (int i = 0; i < num_kerning_entries; ++i)
	{
		const auto& kerning_info = kerning_entries[i].GetObject();
		m_kerning_info[i].additional_spacing = kerning_info["addition_spacing"].GetFloat();
		m_kerning_info[i].previous_glyph = utf8_to_char32(kerning_info["previous_glyph"].GetString());
		m_kerning_info[i].current_glyph = utf8_to_char32(kerning_info["current_glyph"].GetString());
	}
	// </read_kerning()>
}

void font::shutdown()
{

}

asset_type font::get_type() const
{
	return asset_type::font;
}

// private
//////////
