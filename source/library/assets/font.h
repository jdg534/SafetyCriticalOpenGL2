#pragma once // TODO convert to ifndef, define & endif. pragma once isn't in the c++ standard

#include <string>
#include <string_view>

#include "asset.h"
#include "../render/include_opengl.h"

struct glyph_image_info
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
	font(const std::string& name, std::weak_ptr<asset_manager> asset_manager);
	virtual ~font();

	void initialise(std::string_view file_path) override;
	void shutdown() override;
	asset_type get_type() const override;

private:

	std::vector<glyph_image_info> m_glyph_in_image_values;
	std::vector<kerning_info> m_kerning_info;
	std::string m_atlas_asset_name;
	float m_default_spacing = 10.0f; // pixels...
};

