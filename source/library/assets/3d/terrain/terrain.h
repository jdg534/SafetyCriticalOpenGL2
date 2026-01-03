#ifndef _TERRAIN_H_
#define _TERRAIN_H_

#include "../../asset.h"
#include "../../../render/include_opengl.h"

#include <tiffio.h>

#include <string>
#include <vector>

// full spec: http://geotiff.maptools.org/spec/geotiff6.html just focusing on meters and feet for brevity
enum class tiff_pixel_units : uint8_t
{
	METERS,
	FEET
};

struct geo_tiff_height_info
{
	uint32 width = 0;
	uint32 length = 0;

	// Raster format
	uint16 bits_per_sample = 0;
	uint16 sample_format = 0;

	tiff_pixel_units pixel_units = tiff_pixel_units::METERS;

	// Horizontal scale
	float meters_per_pixel_x = 1.0f;
	float meters_per_pixel_z = 1.0f; // length, positive Z points North.
	float pixel_vertical_units_scale = 1.0f;

	// Elevation mapping (dataset-defined)
	bool use_raw_height_value = false;
	float height_min_meters = 0.0;
	float height_max_meters = 100.0;
};

class terrain : public asset
{

public:

	terrain() = delete;
	terrain(const std::string& name, const std::string& path, std::weak_ptr<const asset_manager> asset_manager);
	virtual ~terrain();

	void initialise() override;
	void shutdown() override;
	asset_type get_type() const override;


	const geo_tiff_height_info& get_height_info() const;
	float get_tiff_height_at(uint16 x, uint16 y) const;
	float get_height_range_value_at(uint16 x, uint16 y) const;
	float get_height_at(float x_world_space, float z_world_space) const;

	gl::GLuint get_vertex_array_object_id() const;
	gl::GLuint get_vertex_buffer_id() const;
	gl::GLuint get_index_buffer_id() const;
	gl::GLuint get_num_indices_to_draw() const;

	gl::GLuint get_splat_map_texture_id() const;
	gl::GLuint get_red_channel_mapped_texture_texture_id() const;
	gl::GLuint get_green_channel_mapped_texture_texture_id() const;
	gl::GLuint get_blue_channel_mapped_texture_texture_id() const;
	gl::GLuint get_alpha_channel_mapped_texture_texture_id() const;

private:

	static void read_heights_uint8(std::vector<float>& output_buffer, TIFF* tiff_file);
	static void read_heights_sint8(std::vector<float>& output_buffer, TIFF* tiff_file);
	static void read_heights_f32(std::vector<float>& output_buffer, TIFF* tiff_file);
	static void read_heights_f32_tiled(std::vector<float>& output_buffer, TIFF* tiff_file);
	static void override_nan_values(std::vector<float>& output_buffer);
	static void flip_rows(std::vector<float>& output_buffer, uint32 width, uint32 length);

	size_t get_height_index(uint16 x, uint16 y) const;
	void generate_open_gl_buffers();
	void check_referenced_textures_are_valid();

	gl::GLuint get_texture_id(std::string_view texture_asset_name) const;

	geo_tiff_height_info m_geo_tiff_height_info;
	std::vector<float> m_heights;
	float m_default_tiff_meters_per_pixel = 1.0f;

	gl::GLuint m_vertex_array_object_id = 0;
	gl::GLuint m_vertex_buffer_id = 0;
	gl::GLuint m_index_buffer_id = 0;
	gl::GLuint m_num_indices_to_draw = 0;

	std::string m_splat_map_asset_name;
	std::string m_red_channel_mapped_texture_asset_name;
	std::string m_green_channel_mapped_texture_asset_name;
	std::string m_blue_channel_mapped_texture_asset_name;
	std::string m_alpha_channel_mapped_texture_asset_name;
};

#endif // _TERRAIN_H_
