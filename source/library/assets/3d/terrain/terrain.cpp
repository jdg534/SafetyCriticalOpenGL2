#include "terrain.h"

#include "../../asset_utils.h"

#include "../../../render/vertex_types.h"
#include "../../asset_manager.h"
#include "../../texture.h"

#include <tiffio.h>
#include <geotiff.h>
#include <geokeys.h>
#include <geo_normalize.h>
#include <xtiffio.h>
#include <rapidjson/document.h>

#define _USE_MATH_DEFINES
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL // done for lerp, otherwise remove this.
#include <glm/gtx/compatibility.hpp>

#include <cmath>
#include <math.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>


// public
/////////

ROAM_leaf_node::~ROAM_leaf_node()
{
	if (north_west_child)
	{
		delete north_west_child;
		north_west_child = nullptr;
	}
	if (north_east_child)
	{
		delete north_east_child;
		north_east_child = nullptr;
	}
	if (south_west_child)
	{
		delete south_west_child;
		south_west_child = nullptr;
	}
	if (south_east_child)
	{
		delete south_east_child;
		south_east_child = nullptr;
	}
}

terrain::terrain(const std::string& name, const std::string& path, std::weak_ptr<const asset_manager> asset_manager)
	: asset(name, path, asset_manager)
	, m_splat_map_texture_id(0)
	, m_red_channel_mapped_texture_texture_id(0)
	, m_green_channel_mapped_texture_texture_id(0)
	, m_blue_channel_mapped_texture_texture_id(0)
	, m_alpha_channel_mapped_texture_texture_id(0)
{

}

terrain::~terrain()
{
}

void terrain::initialise()
{
	using namespace std;
	// load the file!
	const string_view path = get_path();
	if (!filesystem::exists(path)) throw exception("Couldn't find file at path");

	ifstream assets_list_file(path.data());
	stringstream buffer;
	buffer << assets_list_file.rdbuf();
	rapidjson::Document doc;
	if (doc.Parse(buffer.str().c_str()).HasParseError())
	{
		throw std::exception("asset_manager::initialise failed to parse the json");
	}
	assets_list_file.close();
	buffer.clear();

	if (!doc.HasMember("height_map"))
	{
		throw std::exception("terrain doesn't have a height map!");
	}
	const string resolved_path = asset_utils::resolve_file_path(doc["height_map"].GetString(), asset_utils::get_directory_path(get_path())); // ensure an absolute path.

	TIFF* tiff_file = XTIFFOpen(resolved_path.data(), "r");
	if (!tiff_file) throw exception("Failed to load tiff file");

	GTIF* geo_tiff = GTIFNew(tiff_file);
	if (!geo_tiff)
		throw std::runtime_error("GTIFNew failed");

	TIFFGetField(tiff_file, TIFFTAG_IMAGEWIDTH, &m_geo_tiff_height_info.width);
	TIFFGetField(tiff_file, TIFFTAG_IMAGELENGTH, &m_geo_tiff_height_info.length);

	// --- Raster size ---
	TIFFGetField(tiff_file, TIFFTAG_BITSPERSAMPLE, &m_geo_tiff_height_info.bits_per_sample);
	TIFFGetFieldDefaulted(tiff_file, TIFFTAG_SAMPLEFORMAT, &m_geo_tiff_height_info.sample_format);


	// we're going to assume the tiff is using angular values (degees) for the X & Y in tiff, not linear (meters)
	// keys are in: geokeys.inc

	// --- Pixel scale ---
	unsigned short num_pixel_scale_count = 0;
	double* pixel_scale = nullptr;
	if (TIFFGetField(tiff_file, TIFFTAG_GEOPIXELSCALE,&num_pixel_scale_count, &pixel_scale))
	{
		assert(pixel_scale != nullptr);

		const float pixel_longitude_scale_in_degrees = pixel_scale[0];
		const float pixel_latitude_scale_in_degrees = pixel_scale[1];

		constexpr float METERS_PER_DEGREE_LATITUDE = 111320.0f;
		const float centre_latitude_degrees = calculate_centre_latitude_from_tiepoints(tiff_file, m_geo_tiff_height_info.length, pixel_latitude_scale_in_degrees);

		const float centre_latitude_radians = centre_latitude_degrees * M_PI / 180.0f;
		const float meters_per_degree_longitude = METERS_PER_DEGREE_LATITUDE * std::cos(centre_latitude_radians);

		m_geo_tiff_height_info.meters_per_pixel_x = pixel_longitude_scale_in_degrees * meters_per_degree_longitude;
		m_geo_tiff_height_info.meters_per_pixel_z = pixel_latitude_scale_in_degrees * METERS_PER_DEGREE_LATITUDE;
		m_geo_tiff_height_info.pixel_vertical_units_scale = num_pixel_scale_count > 2 ? pixel_scale[2] : m_geo_tiff_height_info.pixel_vertical_units_scale;
		m_geo_tiff_height_info.pixel_vertical_units_scale = m_geo_tiff_height_info.pixel_vertical_units_scale == 0.0f ? 1.0f : m_geo_tiff_height_info.pixel_vertical_units_scale;
	}

	// --- Vertical units ---
	short vertical_units = 0;
	if (GTIFKeyGet(geo_tiff, VerticalUnitsGeoKey, &vertical_units, 0, 1))
	{
		// values from: http://geotiff.maptools.org/spec/geotiff6.html, focusing on meters and feet.
		if (vertical_units == 9001)
		{
			m_geo_tiff_height_info.pixel_units = tiff_pixel_units::METERS;
		}
		else if (vertical_units == 9002)
		{
			m_geo_tiff_height_info.pixel_units = tiff_pixel_units::FEET;
		}
		else
		{
			throw runtime_error("Unsupported vertical unit encountered");
		}
	}

	if (m_geo_tiff_height_info.sample_format == SAMPLEFORMAT_VOID) { throw runtime_error("unsupported height data format"); }

	const bool is_tiled = TIFFIsTiled(tiff_file);

	m_heights.resize(m_geo_tiff_height_info.width * m_geo_tiff_height_info.length);

	if (is_tiled)
	{
		if (m_geo_tiff_height_info.sample_format == SAMPLEFORMAT_IEEEFP && m_geo_tiff_height_info.bits_per_sample == 32)
		{
			read_heights_f32_tiled(m_heights, tiff_file);
			m_geo_tiff_height_info.use_raw_height_value = true;
		}
		else
		{
			throw runtime_error("Unsupported tiled tiff file format encountered.");
		}
	}
	else
	{
		if (m_geo_tiff_height_info.sample_format == SAMPLEFORMAT_UINT && m_geo_tiff_height_info.bits_per_sample == 8)
		{
			read_heights_uint8(m_heights, tiff_file);
		}
		else if (m_geo_tiff_height_info.sample_format == SAMPLEFORMAT_INT && m_geo_tiff_height_info.bits_per_sample == 8)
		{
			read_heights_sint8(m_heights, tiff_file);
		}
		else if (m_geo_tiff_height_info.sample_format == SAMPLEFORMAT_IEEEFP && m_geo_tiff_height_info.bits_per_sample == 32)
		{
			read_heights_f32(m_heights, tiff_file);
			m_geo_tiff_height_info.use_raw_height_value = true;
		}
		else
		{
			throw runtime_error("Unsupported tiff file format encountered.");
		}
	}
	flip_rows(m_heights, m_geo_tiff_height_info.width, m_geo_tiff_height_info.length);
	override_nan_values(m_heights);
	GTIFFree(geo_tiff);
	XTIFFClose(tiff_file);
	tiff_file = nullptr;

	if ((doc.HasMember("height_min_meters") && doc.HasMember("height_max_meters")) && false == m_geo_tiff_height_info.use_raw_height_value)
	{
		m_geo_tiff_height_info.height_min_meters = doc["height_min_meters"].GetFloat();
		m_geo_tiff_height_info.height_max_meters = doc["height_max_meters"].GetFloat();
	}

	generate_ROAM_tree();
	generate_open_gl_buffers();

	m_splat_map_texture_id = get_texture_id(doc["splat_map_asset_name"].GetString());
	m_red_channel_mapped_texture_texture_id = get_texture_id(doc["red_channel_mapped_texture_asset_name"].GetString());
	m_green_channel_mapped_texture_texture_id = get_texture_id(doc["green_channel_mapped_texture_asset_name"].GetString());
	m_blue_channel_mapped_texture_texture_id = get_texture_id(doc["blue_channel_mapped_texture_asset_name"].GetString());
	m_alpha_channel_mapped_texture_texture_id = get_texture_id(doc["alpha_channel_mapped_texture_asset_name"].GetString());
}

void terrain::shutdown()
{
	using namespace gl;
	m_heights.clear();
	const size_t n_tiles = m_ROAM_tree.renderable_areas.size();
	std::vector<GLuint> vao_ids(n_tiles);
	std::vector<GLuint> vertex_buffer_ids(n_tiles);
	std::vector<GLuint> index_buffer_ids(n_tiles);
	for (size_t i = 0; i < n_tiles; ++i)
	{
		vao_ids[i] = m_ROAM_tree.renderable_areas[i].vertex_array_object_id;
		vertex_buffer_ids[i] = m_ROAM_tree.renderable_areas[i].vertex_buffer_id;
		index_buffer_ids[i] = m_ROAM_tree.renderable_areas[i].index_buffer_id;
	}
	glDeleteVertexArrays(n_tiles, vao_ids.data());
	glDeleteBuffers(n_tiles, vertex_buffer_ids.data());
	glDeleteBuffers(n_tiles, index_buffer_ids.data());
	vao_ids.clear();
	vertex_buffer_ids.clear();
	index_buffer_ids.clear();
	m_ROAM_tree.renderable_areas.clear();
}

asset_type terrain::get_type() const
{
	return asset_type::terrain;
}

const geo_tiff_height_info& terrain::get_height_info() const
{
	return m_geo_tiff_height_info;
}

float terrain::get_tiff_height_at(uint64 x_tiff_pixels, uint64 y_tiff_pixels) const
{
	const size_t index = get_height_index(x_tiff_pixels, y_tiff_pixels);
	assert(index < m_heights.size());
	return m_heights[index];
}

const std::vector<renderable_tile_area>& terrain::get_renderable_tiles() const
{
	return m_ROAM_tree.renderable_areas;
}

gl::GLuint terrain::get_splat_map_texture_id() const
{
	assert(m_splat_map_texture_id != 0);
	return m_splat_map_texture_id;
}

gl::GLuint terrain::get_red_channel_mapped_texture_texture_id() const
{
	assert(m_red_channel_mapped_texture_texture_id != 0);
	return m_red_channel_mapped_texture_texture_id;
}

gl::GLuint terrain::get_green_channel_mapped_texture_texture_id() const
{
	assert(m_green_channel_mapped_texture_texture_id != 0);
	return m_green_channel_mapped_texture_texture_id;
}

gl::GLuint terrain::get_blue_channel_mapped_texture_texture_id() const
{
	assert(m_blue_channel_mapped_texture_texture_id != 0);
	return m_blue_channel_mapped_texture_texture_id;
}

gl::GLuint terrain::get_alpha_channel_mapped_texture_texture_id() const
{
	assert(m_alpha_channel_mapped_texture_texture_id != 0);
	return m_alpha_channel_mapped_texture_texture_id;
}

// private
//////////

void terrain::read_heights_uint8(std::vector<float>& output_buffer, TIFF* tiff_file)
{
	uint32 width = 0, height = 0;
	TIFFGetField(tiff_file, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(tiff_file, TIFFTAG_IMAGELENGTH, &height);

	const tsize_t scanline_size = TIFFScanlineSize(tiff_file);
	std::vector<uint8_t> scanline(scanline_size);

	constexpr float INV_255 = 1.0f / 255.0f;
	for (uint32 row = 0; row < height; ++row)
	{
		if (TIFFReadScanline(tiff_file, scanline.data(), row) != 1) throw std::runtime_error("TIFFReadScanline failed");
		float* dst = output_buffer.data() + row * width;
		for (uint32 col = 0; col < width; ++col)
		{
			dst[col] = static_cast<float>(scanline[col]) * INV_255;
		}
	}
}

void terrain::read_heights_sint8(std::vector<float>& output_buffer, TIFF* tiff_file)
{
	uint32 width = 0, height = 0;
	TIFFGetField(tiff_file, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(tiff_file, TIFFTAG_IMAGELENGTH, &height);

	const tsize_t scanline_size = TIFFScanlineSize(tiff_file);
	std::vector<int8_t> scanline(scanline_size);

	constexpr float INV_128 = 1.0f / 128.0f;
	for (uint32 row = 0; row < height; ++row)
	{
		if (TIFFReadScanline(tiff_file, scanline.data(), row) != 1) throw std::runtime_error("TIFFReadScanline failed");
		float* dst = output_buffer.data() + row * width;
		for (uint32 col = 0; col < width; ++col)
		{
			dst[col] = static_cast<float>(scanline[col]) * INV_128;
			dst[col] = (dst[col] + 1.0f) / 2.0f;
		}
	}
}

void terrain::read_heights_f32(std::vector<float>& output_buffer, TIFF* tiff_file)
{
	uint32 width = 0, height = 0;
	TIFFGetField(tiff_file, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(tiff_file, TIFFTAG_IMAGELENGTH, &height);
	const tsize_t scanline_size_bytes = TIFFScanlineSize(tiff_file);
	std::vector<float> scanline(scanline_size_bytes / sizeof(float));

	for (uint32 row = 0; row < height; ++row)
	{
		if (TIFFReadScanline(tiff_file, scanline.data(), row) != 1) throw std::runtime_error("TIFFReadScanline failed");

		float* dst = output_buffer.data() + row * width;
		std::memcpy(dst, scanline.data(), width * sizeof(float));
	}
}

void terrain::read_heights_f32_tiled(std::vector<float>& output_buffer, TIFF* tiff_file)
{
	uint32 image_width = 0, image_height = 0;
	uint32 tile_width = 0, tile_height = 0;

	TIFFGetField(tiff_file, TIFFTAG_IMAGEWIDTH, &image_width);
	TIFFGetField(tiff_file, TIFFTAG_IMAGELENGTH, &image_height);
	TIFFGetField(tiff_file, TIFFTAG_TILEWIDTH, &tile_width);
	TIFFGetField(tiff_file, TIFFTAG_TILELENGTH, &tile_height);

	const tsize_t tile_size_bytes = TIFFTileSize(tiff_file);
	std::vector<float> tile(tile_size_bytes / sizeof(float));

	for (uint32 tile_y = 0; tile_y < image_height; tile_y += tile_height)
	{
		for (uint32 tile_x = 0; tile_x < image_width; tile_x += tile_width)
		{
			if (TIFFReadTile(tiff_file, tile.data(), tile_x, tile_y, 0, 0) == -1) throw std::runtime_error("TIFFReadTile failed");
			const uint32 max_y = std::min(tile_height, image_height - tile_y);
			const uint32 max_x = std::min(tile_width, image_width - tile_x);
			for (uint32 y = 0; y < max_y; ++y)
			{
				float* dst = output_buffer.data()
					+ (tile_y + y) * image_width
					+ tile_x;
				const float* src = tile.data() + y * tile_width;
				std::memcpy(dst, src, max_x * sizeof(float));
			}
		}
	}
}

void terrain::override_nan_values(std::vector<float>& output_buffer)
{
	const size_t n_elements = output_buffer.size();
	assert(n_elements > 0);
	float last_good_value = output_buffer[0];
	for (size_t i = 0; i < n_elements; ++i)
	{
		if (!std::isnan(output_buffer[i]))
		{
			last_good_value = output_buffer[i];
		}
		else
		{
			output_buffer[i] = last_good_value;
		}
	}
}

void terrain::flip_rows(std::vector<float>& output_buffer, uint32 width, uint32 length)
{
	if (width == 0 || length == 0)
		return;

	if (output_buffer.size() != static_cast<size_t>(width) * length)
		throw std::runtime_error("Invalid heightmap buffer size");

	const size_t row_elements = width;
	const size_t row_bytes = row_elements * sizeof(float);
	const size_t half_height = length / 2;

	std::vector<float> temp(row_elements);

	for (size_t row = 0; row < half_height; ++row)
	{
		const size_t opposite = (length - 1) - row;

		float* a = output_buffer.data() + row * row_elements;
		float* b = output_buffer.data() + opposite * row_elements;

		std::memcpy(temp.data(), a, row_bytes);
		std::memcpy(a, b, row_bytes);
		std::memcpy(b, temp.data(), row_bytes);
	}
}

float terrain::calculate_centre_latitude_from_tiepoints(TIFF* tiff_file, uint32 image_height, float pixel_latitude_scale_degrees)
{
	double* tiepoints = nullptr;
	uint16 tiepoint_count = 0;

	if (!TIFFGetField(tiff_file, TIFFTAG_GEOTIEPOINTS, &tiepoint_count, &tiepoints))
		throw std::runtime_error("GeoTIFF has no tiepoints");

	if (tiepoint_count < 6)
		throw std::runtime_error("Invalid GeoTIFF tiepoint data");

	// First tiepoint (usually top-left)
	const double tie_latitude = tiepoints[4]; // Y

	return tie_latitude - (image_height * 0.5 * pixel_latitude_scale_degrees);
}

size_t terrain::get_height_index(uint32 x_tiff_pixels, uint32 y_tiff_pixels) const
{
	size_t results = (m_geo_tiff_height_info.width * y_tiff_pixels) + x_tiff_pixels;
	assert(results >= y_tiff_pixels && results>= x_tiff_pixels);
	return results;
}

void terrain::generate_ROAM_tree()
{
	const geo_tiff_height_info& height_info = get_height_info();

	assert(m_ROAM_tree.root == nullptr);
	m_ROAM_tree.root = new ROAM_leaf_node;
	m_ROAM_tree.root->north_tiff_px = 0;
	m_ROAM_tree.root->west_tiff_px = 0;
	m_ROAM_tree.root->south_tiff_px = height_info.length - 1; // remember we subdivide on pixels, 0 indexed.
	m_ROAM_tree.root->east_tiff_px = height_info.width - 1; // (as above)
	generate_ROAM_tree_worker(m_ROAM_tree.root);
}

void terrain::generate_ROAM_tree_worker(ROAM_leaf_node* current_leaf) const
{
	const uint32 width = current_leaf->east_tiff_px - current_leaf->west_tiff_px;
	const uint32 length = current_leaf->south_tiff_px - current_leaf->north_tiff_px;
	const bool can_subdivide = width > 3 && length > 3;
	const float vertical_delta = calculate_vertical_delta_for_leaf(current_leaf);
	if (can_subdivide && vertical_delta > m_ROAM_tree.vertical_delta_to_stop_recursion_at)
	{
		const float flt_north_tiff_px = static_cast<float>(current_leaf->north_tiff_px);
		const float flt_south_tiff_px = static_cast<float>(current_leaf->south_tiff_px);
		const float flt_west_tiff_px = static_cast<float>(current_leaf->west_tiff_px);
		const float flt_east_tiff_px = static_cast<float>(current_leaf->east_tiff_px);
		const uint32 west_to_east_mid_point = static_cast<uint32>(std::floorf(glm::lerp(flt_west_tiff_px, flt_east_tiff_px,  0.5f)));
		const uint32 north_to_south_mid_point = static_cast<uint32>(std::floorf(glm::lerp(flt_north_tiff_px, flt_south_tiff_px, 0.5f)));
		
		current_leaf->north_west_child = new ROAM_leaf_node;
		current_leaf->north_west_child->north_tiff_px = current_leaf->north_tiff_px;
		current_leaf->north_west_child->west_tiff_px = current_leaf->west_tiff_px;
		current_leaf->north_west_child->south_tiff_px = north_to_south_mid_point;
		current_leaf->north_west_child->east_tiff_px = west_to_east_mid_point;
		
		current_leaf->north_east_child = new ROAM_leaf_node;
		current_leaf->north_east_child->north_tiff_px = current_leaf->north_tiff_px;
		current_leaf->north_east_child->west_tiff_px = west_to_east_mid_point;
		current_leaf->north_east_child->south_tiff_px = north_to_south_mid_point;
		current_leaf->north_east_child->east_tiff_px = current_leaf->east_tiff_px;
		
		current_leaf->south_west_child= new ROAM_leaf_node;
		current_leaf->south_west_child->north_tiff_px = north_to_south_mid_point;
		current_leaf->south_west_child->west_tiff_px = current_leaf->west_tiff_px;
		current_leaf->south_west_child->south_tiff_px = current_leaf->south_tiff_px;
		current_leaf->south_west_child->east_tiff_px = west_to_east_mid_point;
		
		current_leaf->south_east_child = new ROAM_leaf_node;
		current_leaf->south_east_child->north_tiff_px = north_to_south_mid_point;
		current_leaf->south_east_child->west_tiff_px = west_to_east_mid_point;
		current_leaf->south_east_child->south_tiff_px = current_leaf->south_tiff_px;
		current_leaf->south_east_child->east_tiff_px = current_leaf->east_tiff_px;
		
		generate_ROAM_tree_worker(current_leaf->north_west_child);
		generate_ROAM_tree_worker(current_leaf->north_east_child);
		generate_ROAM_tree_worker(current_leaf->south_west_child);
		generate_ROAM_tree_worker(current_leaf->south_east_child);
	}
}

float terrain::calculate_vertical_delta_for_leaf(const ROAM_leaf_node* const leaf) const
{
	float min_value = std::numeric_limits<float>::max();
	float max_value = std::numeric_limits<float>::lowest();
	for (uint32 i = leaf->north_tiff_px; i <= leaf->south_tiff_px; ++i)
	{
		for (uint32 j = leaf->west_tiff_px; j <= leaf->east_tiff_px; ++j)
		{
			const float px_height = get_tiff_height_at(j, i);
			min_value = std::min(px_height, min_value);
			max_value = std::max(px_height, max_value);
		}
	}
	return std::abs(max_value - min_value);
}

void terrain::populate_buffers(const ROAM_leaf_node* const leaf,
	std::vector<std::vector<vertex_types::terrain_vertex>>& out_vb, std::vector<std::vector<uint16>>& out_ib)
{
	if (does_leaf_have_children(leaf))
	{
		populate_buffers(leaf->north_west_child, out_vb, out_ib);
		populate_buffers(leaf->north_east_child, out_vb, out_ib);
		populate_buffers(leaf->south_west_child, out_vb, out_ib);
		populate_buffers(leaf->south_east_child, out_vb, out_ib);
	}
	else
	{
		if (out_vb.size() == 0 || out_ib.size() == 0)
		{
			out_vb.push_back({});
			out_ib.push_back({});
		}

		const size_t current_vb_size = out_vb[out_vb.size() - 1].size();
		if (current_vb_size + 4 >= MAX_VERTEX_BUFFER_SIZE)
		{
			out_vb.push_back({});
			out_ib.push_back({});
		}

		std::vector<vertex_types::terrain_vertex>& vb = out_vb[out_vb.size() - 1];
		std::vector<uint16>& ib = out_ib[out_ib.size() - 1];

		const size_t pre_insert_vertex_buffer_size = vb.size();

		vb.push_back(get_vertex_for_tiff_pixel(leaf->west_tiff_px, leaf->north_tiff_px));
		vb.push_back(get_vertex_for_tiff_pixel(leaf->east_tiff_px, leaf->north_tiff_px));
		vb.push_back(get_vertex_for_tiff_pixel(leaf->west_tiff_px, leaf->south_tiff_px));
		vb.push_back(get_vertex_for_tiff_pixel(leaf->east_tiff_px, leaf->south_tiff_px));


		const uint32 north_west_vert_index = pre_insert_vertex_buffer_size + 0;
		const uint32 north_east_vert_index = pre_insert_vertex_buffer_size + 1;
		const uint32 south_west_vert_index = pre_insert_vertex_buffer_size + 2;
		const uint32 south_east_vert_index = pre_insert_vertex_buffer_size + 3;

		assert(north_west_vert_index >= pre_insert_vertex_buffer_size);
		assert(north_east_vert_index >= pre_insert_vertex_buffer_size);
		assert(south_west_vert_index >= pre_insert_vertex_buffer_size);
		assert(south_east_vert_index >= pre_insert_vertex_buffer_size);

		ib.push_back(north_west_vert_index);
		ib.push_back(south_west_vert_index);
		ib.push_back(north_east_vert_index);

		ib.push_back(north_east_vert_index);
		ib.push_back(south_west_vert_index);
		ib.push_back(south_east_vert_index);
	}
}

void terrain::sanity_check_buffer_data(const std::vector<vertex_types::terrain_vertex>& vertex_buffer_data, const std::vector<uint16>& index_buffer_data)
{
	using namespace vertex_types;
	const size_t vb_size = vertex_buffer_data.size();
	assert(vb_size > 0);
	assert(vb_size < MAX_VERTEX_BUFFER_SIZE);
	for (const terrain_vertex& vertex : vertex_buffer_data)
	{
		constexpr glm::vec3 origin{ 0.0f,0.0f,0.0f };
		assert(vertex.position != origin);
		assert(!glm::any(glm::isnan(vertex.position)));
		assert(!glm::any(glm::isnan(vertex.texture_coordinates)));
		assert(!glm::any(glm::isnan(vertex.normal)));
		assert(!glm::any(glm::isnan(vertex.terrain_texture_coordinates)));
	}
	assert(index_buffer_data.size() % 6 == 0);
	for (const uint16 index : index_buffer_data)
	{
		assert(!std::isnan(index));
		assert(index >= 0);
		assert(index < vb_size);
	}
}


bool terrain::does_leaf_have_children(const ROAM_leaf_node* const leaf)
{
	if (leaf->north_west_child) return true;
	if (leaf->north_east_child) return true;
	if (leaf->south_west_child) return true;
	if (leaf->south_east_child) return true;
	return false;
}

void terrain::generate_open_gl_buffers()
{
	using namespace std;
	using namespace vertex_types;
	using namespace gl;

	std::vector<std::vector<vertex_types::terrain_vertex>> vertex_buffers;
	std::vector<std::vector<uint16>> index_buffers;
	populate_buffers(m_ROAM_tree.root, vertex_buffers, index_buffers);

	const size_t num_buffers_needed_for_ROAM = vertex_buffers.size();
	m_ROAM_tree.renderable_areas.resize(num_buffers_needed_for_ROAM);

	GLuint* ROAM_vao_ids = new GLuint[num_buffers_needed_for_ROAM];
	glGenVertexArrays(num_buffers_needed_for_ROAM, ROAM_vao_ids);
	GLuint* ROAM_vertex_buffer_ids = new GLuint[num_buffers_needed_for_ROAM];
	glGenBuffers(num_buffers_needed_for_ROAM, ROAM_vertex_buffer_ids);
	GLuint* ROAM_index_buffer_ids = new GLuint[num_buffers_needed_for_ROAM];
	glGenBuffers(num_buffers_needed_for_ROAM, ROAM_index_buffer_ids);

	for (size_t i = 0; i < num_buffers_needed_for_ROAM; ++i)
	{
		m_ROAM_tree.renderable_areas[i].tile_index = i;
		m_ROAM_tree.renderable_areas[i].vertex_buffer_id = ROAM_vertex_buffer_ids[i];
		m_ROAM_tree.renderable_areas[i].vertex_array_object_id = ROAM_vao_ids[i];
		m_ROAM_tree.renderable_areas[i].index_buffer_id = ROAM_index_buffer_ids[i];

		set_tile_bounds(vertex_buffers[i], m_ROAM_tree.renderable_areas[i]);

		setup_vertex_attrib_array(m_ROAM_tree.renderable_areas[i].vertex_array_object_id);
		glBindBuffer(GL_ARRAY_BUFFER, m_ROAM_tree.renderable_areas[i].vertex_buffer_id);
		glBufferData(GL_ARRAY_BUFFER, vertex_buffers[i].size() * terrain_vertex_struct_size, vertex_buffers[i].data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ROAM_tree.renderable_areas[i].index_buffer_id);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffers[i].size() * sizeof(uint16), index_buffers[i].data(), GL_STATIC_DRAW);
		m_ROAM_tree.renderable_areas[i].num_indices_to_draw = index_buffers[i].size();
	}
	delete[] ROAM_vao_ids;
	delete[] ROAM_vertex_buffer_ids;
	delete[] ROAM_index_buffer_ids;
	
	// Deleteing the tree. we've got what we want of out it.
	delete m_ROAM_tree.root; // remember the delete operator will recursively delete 

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

vertex_types::terrain_vertex terrain::get_vertex_for_tiff_pixel(uint64 x_tiff_pixels, uint64 y_tiff_pixels) const
{
	vertex_types::terrain_vertex result;
	std::memset(&result, 0, vertex_types::terrain_vertex_struct_size);

	const float tiff_width_as_float = static_cast<float>(m_geo_tiff_height_info.width); // of the entire file in px
	const float tiff_length_as_float = static_cast<float>(m_geo_tiff_height_info.length); // of the entire file in px

	// the top north west corner
	const float far_west = -(tiff_width_as_float * 0.5f) * m_geo_tiff_height_info.meters_per_pixel_x;
	const float far_north = -(static_cast<float>(m_geo_tiff_height_info.length) * 0.5f) * m_geo_tiff_height_info.meters_per_pixel_z;

	const float x_tiff_pixels_as_float = static_cast<float>(x_tiff_pixels);
	const float y_tiff_pixels_as_float = static_cast<float>(y_tiff_pixels);

	const bool got_left_px = x_tiff_pixels > 0;
	const bool got_above_px = y_tiff_pixels > 0;
	const bool got_right_px = (x_tiff_pixels + 1) < m_geo_tiff_height_info.width;
	const bool got_below_px = (y_tiff_pixels + 1) < m_geo_tiff_height_info.length;

	const uint64 left_px = got_left_px ? x_tiff_pixels - 1 : x_tiff_pixels;
	const uint64 above_px = got_above_px ? y_tiff_pixels - 1 : y_tiff_pixels;
	const uint64 right_px = got_right_px ? x_tiff_pixels + 1 : x_tiff_pixels;
	const uint64 below_px = got_below_px ? y_tiff_pixels + 1 : y_tiff_pixels;

	const float current_px_height = get_tiff_height_at(x_tiff_pixels, y_tiff_pixels);
	const float above_px_height = get_tiff_height_at(x_tiff_pixels, above_px);
	const float left_px_height = get_tiff_height_at(left_px, y_tiff_pixels);
	const float right_px_height = get_tiff_height_at(right_px, y_tiff_pixels);
	const float below_px_height = get_tiff_height_at(x_tiff_pixels, below_px);

	result.position.x = far_west + (x_tiff_pixels * m_geo_tiff_height_info.meters_per_pixel_x);
	result.position.y = current_px_height;
	result.position.z = far_north + (y_tiff_pixels * m_geo_tiff_height_info.meters_per_pixel_z);
	result.texture_coordinates.x = x_tiff_pixels_as_float / tiff_width_as_float;
	result.texture_coordinates.y = 1.0f - (y_tiff_pixels_as_float / tiff_length_as_float);

	result.terrain_texture_coordinates.x = x_tiff_pixels_as_float;
	result.terrain_texture_coordinates.y = tiff_length_as_float - y_tiff_pixels_as_float;

	const glm::vec3 dx = glm::vec3(2.0f * m_geo_tiff_height_info.meters_per_pixel_x, right_px_height - left_px_height, 0.0f);
	const glm::vec3 dy = glm::vec3(0.0f, above_px_height - below_px_height, 2.0f * m_geo_tiff_height_info.meters_per_pixel_z);
	result.normal = glm::normalize(glm::cross(dy, dx));
	if (glm::any(glm::isnan(result.normal))) result.normal = glm::vec3(0, 1, 0);
	return result;
}

void terrain::set_tile_bounds(const std::vector<vertex_types::terrain_vertex>& vertices, renderable_tile_area& to_set)
{
	to_set.north_edge_in_meters     = -FLT_MAX;
	to_set.south_edge_in_meters     =  FLT_MAX;
	to_set.west_edge_in_meters      =  FLT_MAX;
	to_set.east_edge_in_meters      = -FLT_MAX;
	to_set.heighest_point_in_meters = -FLT_MAX;
	to_set.lowest_point_in_meters   =  FLT_MAX;

	for (const auto& vertex : vertices)
	{
		to_set.north_edge_in_meters = std::max(to_set.north_edge_in_meters, vertex.position.z);
		to_set.south_edge_in_meters = std::min(to_set.south_edge_in_meters, vertex.position.z);
		to_set.west_edge_in_meters = std::min(to_set.west_edge_in_meters, vertex.position.x);
		to_set.east_edge_in_meters = std::max(to_set.east_edge_in_meters, vertex.position.x);
		to_set.heighest_point_in_meters = std::max(to_set.heighest_point_in_meters, vertex.position.y);
		to_set.lowest_point_in_meters = std::min(to_set.lowest_point_in_meters, vertex.position.y);
	}
}

void terrain::setup_vertex_attrib_array(gl::GLuint vertex_attrib_array_id)
{
	using namespace gl;
	using namespace vertex_types;
	glBindVertexArray(vertex_attrib_array_id);
	// attribute layout: 0 = position, 1 = texture_coordinates ,2 = normal, 3 = terrain_uv
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, terrain_vertex_struct_size, (void*)offsetof(terrain_vertex, position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, terrain_vertex_struct_size, (void*)offsetof(terrain_vertex, texture_coordinates));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, terrain_vertex_struct_size, (void*)offsetof(terrain_vertex, normal));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, terrain_vertex_struct_size, (void*)offsetof(terrain_vertex, terrain_texture_coordinates));
}

gl::GLuint terrain::get_texture_id(std::string_view texture_asset_name) const
{
	std::weak_ptr<const asset> result = get_asset_manager().lock()->get_asset_on_name(texture_asset_name);
	return std::dynamic_pointer_cast<const texture>(result.lock())->get_id();
}