#ifndef _TERRAIN_H_
#define _TERRAIN_H_

#include "../../asset.h"
#include "../../../render/include_opengl.h"
#include "../../../render/vertex_types.h"

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
	float re_tile_squared_tile_distance_in_meters = 500.0f; // we'll want to convert to using a 16 bit index buffer later.
	// might be worth having a way to auto calculate for bigest cell size at load time. only need 4 index buffers

	// Horizontal scale
	float meters_per_pixel_x = 1.0f;
	float meters_per_pixel_z = 1.0f; // length, positive Z points North.
	float pixel_vertical_units_scale = 1.0f;

	// the following are only to be used if raw values can't be used for height.
	bool use_raw_height_value = false;
	float height_min_meters = 0.0;
	float height_max_meters = 100.0;
};

// use the same buffer 

struct renderable_tile_area
{
	float north_edge_in_meters = 0.0f;
	float south_edge_in_meters = 0.0f;
	float west_edge_in_meters = 0.0f;
	float east_edge_in_meters = 0.0f;
	float heighest_point_in_meters = 0.0f;

	int32 tile_index = -1;
	gl::GLuint vertex_buffer_id = 0;
	gl::GLuint index_buffer_id = 0; // yet it if we want 
	gl::GLuint vertex_array_object_id = 0;
	gl::GLuint num_indices_to_draw = 0;
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

	const std::vector<renderable_tile_area>& get_renderable_tiles() const;

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
	static float calculate_centre_latitude_from_tiepoints(TIFF* tiff_file, uint32 image_height, float pixel_latitude_scale_degrees);

	size_t get_height_index(uint16 x, uint16 y) const;
	void generate_open_gl_buffers();

	std::vector<vertex_types::vertex_3d> generate_vertex_buffer_data(uint32 tiff_north_px, uint32 tiff_south_px, uint32 tiff_west_px, uint32 tiff_east_px) const;
	static std::vector<uint32_t> generate_index_buffer_data(uint32 width, uint32 length); // todo: see if can change to uint16 later

	static void setup_vertex_attrib_array(gl::GLuint vertex_attrib_array_id);
	void check_referenced_textures_are_valid();

	gl::GLuint get_texture_id(std::string_view texture_asset_name) const;

	geo_tiff_height_info m_geo_tiff_height_info;
	std::vector<float> m_heights;

	// refactor into different buffers. We want a grid of areas so we don't draw all of the terrain in one go.
	// split on camera far distance.

	std::vector<renderable_tile_area> m_renderable_tiles;
	// [0] = normal index buffer ID
	// [1] = east edge index buffer ID
	// [2] = south edge index buffer ID
	// [3] = south east corner edge index buffer ID
	gl::GLuint m_index_buffer_ids[4];

	// cache the ids! refactor these out later
	std::string m_splat_map_asset_name;
	std::string m_red_channel_mapped_texture_asset_name;
	std::string m_green_channel_mapped_texture_asset_name;
	std::string m_blue_channel_mapped_texture_asset_name;
	std::string m_alpha_channel_mapped_texture_asset_name;

	gl::GLuint m_splat_map_texture_id;
	gl::GLuint m_red_channel_mapped_texture_texture_id;
	gl::GLuint m_green_channel_mapped_texture_texture_id;
	gl::GLuint m_blue_channel_mapped_texture_texture_id;
	gl::GLuint m_alpha_channel_mapped_texture_texture_id;
};

#endif // _TERRAIN_H_
