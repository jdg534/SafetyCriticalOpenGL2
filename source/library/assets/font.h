#pragma once // TODO convert to ifndef, define & endif. pragma once isn't in the c++ standard

#include <string>
#include <string_view>

#include <rapidjson/document.h>

#include "asset.h"
#include "texture.h"
#include "../render/include_opengl.h"

struct glyph_info
{
	char32_t glyph;
	int top_px;
	int bottom_px;
	int left_px;
	int right_px;
};

struct kerning_info
{
	char32_t previous_glyph;
	char32_t current_glyph;
	float additional_spacing;
};

struct source_rect // refactor this to be somewhere more generic later if needed
{
	float top;
	float bottom;
	float left;
	float right;
};

class font : public asset
{
public:

	font() = delete;
	font(const std::string& name, std::weak_ptr<const asset_manager> asset_manager);
	virtual ~font();

	void initialise(std::string_view file_path) override;
	void shutdown() override;
	asset_type get_type() const override;

	bool is_string_supported(const std::u32string& to_check) const;
	int get_character_height() const;
	glyph_info get_glyph_info(char32_t glyph) const;
	kerning_info get_kerning_info(char32_t previous_glyph, char32_t current_glyph) const;
	source_rect get_texture_coordinates_for_glyph(char32_t glyph) const;
	std::weak_ptr<texture> get_texture() const;

private:

	void initialise_glyph_info(const rapidjson::Document& font_file);
	void initialise_kerning_info(const rapidjson::Document& font_file);
	bool does_contain_white_space_character() const;

	std::vector<glyph_info> m_glyph_info;
	std::vector<kerning_info> m_kerning_info;
	std::string m_atlas_asset_name;
	float m_default_spacing = 10.0f; // pixels...
};

