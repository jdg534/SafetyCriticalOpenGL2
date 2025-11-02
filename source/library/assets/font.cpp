#include "font.h"

#include "../utilities/text_utilities.h"
#include "asset_manager.h"

#include <algorithm>
#include <fstream>
#include <string_view>
#include <sstream>

#include <rapidjson/document.h>
#include <rapidjson/encodings.h>

using namespace rapidjson;
using namespace std;

// public
/////////

font::font(const std::string& name, const std::string& path, std::weak_ptr<const asset_manager> asset_manager)
	: asset(name, path, asset_manager)
{

}

font::~font()
{

}

void font::initialise()
{
	std::ifstream assets_list_file(get_path().data());
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

	if (!does_contain_white_space_character())
	{
		throw runtime_error("font does not contain a white space character.");
	}
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

bool font::is_string_supported(const std::u32string& to_check) const
{
	// checks all characters are supported by the font.
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

int font::get_character_height() const
{
	int max_height = 0;
	for (const auto& glyph : m_glyph_info)
	{
		const int glyph_height = glyph.bottom_px - glyph.top_px;
		max_height = max_height < glyph_height ? glyph_height : max_height;
	}
	return max_height;
}

glyph_info font::get_glyph_info(char32_t glyph) const
{
	auto iter = find_if(cbegin(m_glyph_info), cend(m_glyph_info), [=](const glyph_info& info) { return info.glyph == glyph; });
	if (iter != m_glyph_info.end())
	{
		return *iter;
	}
	return get_glyph_info(' '); // this character is expected to be present.
}

kerning_info font::get_kerning_info(char32_t previous_glyph, char32_t current_glyph) const
{
	auto iter = find_if(begin(m_kerning_info), end(m_kerning_info), [=](const kerning_info& info)
		{
			return previous_glyph == info.previous_glyph
				&& current_glyph == info.current_glyph;
		});
	if (iter != m_kerning_info.end())
	{
		return *iter;
	}
	return kerning_info{0,0,0}; // default.
}

source_rect font::get_texture_coordinates_for_glyph(char32_t glyph) const
{
	const glyph_info info = get_glyph_info(glyph);
	const auto texture = get_texture();
	source_rect result;
	const float atlas_width = static_cast<float>(texture.lock()->get_width());
	const float atlas_height = static_cast<float>(texture.lock()->get_height());
	result.left = info.left_px / atlas_width;
	result.right = info.right_px / atlas_width;
	result.top = info.top_px / atlas_height;
	result.bottom = info.bottom_px / atlas_height;
	return result;
}

weak_ptr<texture> font::get_texture() const
{
	return dynamic_pointer_cast<texture>(get_asset_manager().lock()->get_asset_on_name(m_atlas_asset_name).lock());
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

bool font::does_contain_white_space_character() const
{
	return any_of(begin(m_glyph_info), end(m_glyph_info), [](const glyph_info& info)
		{
			return text_utilities::is_character_white_space(info.glyph);
		});
}
