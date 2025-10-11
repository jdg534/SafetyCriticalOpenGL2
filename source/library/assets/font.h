#pragma once // TODO convert to ifndef, define & endif. pragma once isn't in the c++ standard

#include <string>
#include <string_view>

#include <rapidjson/document.h>

#include "asset.h"
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

class font : public asset
{
public:

	font() = delete;
	font(const std::string& name, std::weak_ptr<const asset_manager> asset_manager);
	virtual ~font();

	void initialise(std::string_view file_path) override;
	void shutdown() override;
	asset_type get_type() const override;

	bool is_string_supported(const std::vector<char32_t>& to_check) const;

private:

	void initialise_glyph_info(const rapidjson::Document& font_file);
	void initialise_kerning_info(const rapidjson::Document& font_file);

	std::vector<glyph_info> m_glyph_info;
	std::vector<kerning_info> m_kerning_info;
	std::string m_atlas_asset_name;
	float m_default_spacing = 10.0f; // pixels...
};

