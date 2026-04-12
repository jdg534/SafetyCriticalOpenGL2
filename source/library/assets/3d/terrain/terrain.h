#ifndef _TERRAIN_H_
#define _TERRAIN_H_

#include "../../asset.h"
#include "../../../render/include_opengl.h"
#include "../../../render/vertex_types.h"

#include <tiffio.h>
#include <geotiff.h>

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
};

struct renderable_tile_area
{
	float north_edge_in_meters = 0.0f;
	float south_edge_in_meters = 0.0f;
	float west_edge_in_meters = 0.0f;
	float east_edge_in_meters = 0.0f;
	float heighest_point_in_meters = 0.0f;
	float lowest_point_in_meters = 0.0f;

	int32 tile_index = -1;
	gl::GLuint vertex_buffer_id = 0;
	gl::GLuint index_buffer_id = 0;
	gl::GLuint vertex_array_object_id = 0;
	gl::GLuint num_indices_to_draw = 0;

	glm::vec4 blend_colour { 1.0f, 1.0f, 1.0f, 1.0f };
};

struct ROAM_leaf_node
{
	uint32 north_tiff_px = 0;
	uint32 south_tiff_px = 0;
	uint32 west_tiff_px = 0;
	uint32 east_tiff_px = 0;

	ROAM_leaf_node* north_west_child = nullptr;
	ROAM_leaf_node* north_east_child = nullptr;
	ROAM_leaf_node* south_west_child = nullptr;
	ROAM_leaf_node* south_east_child = nullptr;

	~ROAM_leaf_node();
};

struct ROAM_tree
{
	ROAM_leaf_node* root = nullptr;
	float vertical_delta_to_stop_recursion_at = 0.001f; // meters...

	std::vector<renderable_tile_area> renderable_areas;
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
	float get_tiff_height_at(uint32 x_tiff_pixels, uint32 y_tiff_pixels) const;

	const std::vector<renderable_tile_area>& get_renderable_tiles() const;

	gl::GLuint get_splat_map_texture_id() const;
	gl::GLuint get_red_channel_mapped_texture_texture_id() const;
	gl::GLuint get_green_channel_mapped_texture_texture_id() const;
	gl::GLuint get_blue_channel_mapped_texture_texture_id() const;
	gl::GLuint get_alpha_channel_mapped_texture_texture_id() const;

private:

	void compute_tiff_pixel_dimensions(GTIF* gtif, TIFF* tiff); // args should be const, but the api needs them to be mutable.

	void read_heights(TIFF* tiff_file);
	static void read_heights_uint8(std::vector<float>& output_buffer, TIFF* tiff_file);
	static void read_heights_sint8(std::vector<float>& output_buffer, TIFF* tiff_file);
	static void read_heights_f32(std::vector<float>& output_buffer, TIFF* tiff_file);
	static void read_heights_f32_tiled(std::vector<float>& output_buffer, TIFF* tiff_file);
	static void override_nan_values(std::vector<float>& output_buffer);
	static void flip_rows(std::vector<float>& output_buffer, uint32 width, uint32 length);
	static float calculate_centre_latitude_from_tiepoints(TIFF* tiff_file, uint32 image_height, float pixel_latitude_scale_degrees);

	size_t get_height_index(uint32 x_tiff_pixels, uint32 y_tiff_pixels) const;
	void generate_ROAM_tree();
	void generate_ROAM_tree_worker(ROAM_leaf_node* current_leaf) const;

	float calculate_vertical_delta_for_leaf(const ROAM_leaf_node* const leaf) const;
	void populate_buffers(const ROAM_leaf_node* const leaf,
		std::vector<std::vector<vertex_types::terrain_vertex>>& output_vertex_buffers, std::vector<std::vector<uint16>>& output_index_buffers);

	void sanity_check_buffer_data(const std::vector<vertex_types::terrain_vertex>& vertex_buffer_data, const std::vector<uint16>& index_buffer_data); // debug code
	static bool does_leaf_have_children(const ROAM_leaf_node* const leaf);

	void generate_open_gl_buffers();

	vertex_types::terrain_vertex get_vertex_for_tiff_pixel(uint64 x_tiff_pixels, uint64 y_tiff_pixels) const;
	
	static void set_tile_bounds(const std::vector<vertex_types::terrain_vertex>& vertices, renderable_tile_area& to_set);
	static void setup_vertex_attrib_array(gl::GLuint vertex_attrib_array_id);

	gl::GLuint get_texture_id(std::string_view texture_asset_name) const;

	geo_tiff_height_info m_geo_tiff_height_info;
	std::vector<float> m_heights;

	ROAM_tree m_ROAM_tree;

	gl::GLuint m_splat_map_texture_id;
	gl::GLuint m_red_channel_mapped_texture_texture_id;
	gl::GLuint m_green_channel_mapped_texture_texture_id;
	gl::GLuint m_blue_channel_mapped_texture_texture_id;
	gl::GLuint m_alpha_channel_mapped_texture_texture_id;

	static constexpr size_t MAX_VERTEX_BUFFER_SIZE = std::numeric_limits<uint16>::max();
};

#endif // _TERRAIN_H_
