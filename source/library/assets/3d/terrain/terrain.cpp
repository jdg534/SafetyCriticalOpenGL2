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

terrain::terrain(const std::string& name, const std::string& path, std::weak_ptr<const asset_manager> asset_manager)
	: asset(name, path, asset_manager)
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

	generate_open_gl_buffers();

	m_splat_map_asset_name = doc["splat_map_asset_name"].GetString();
	m_red_channel_mapped_texture_asset_name = doc["red_channel_mapped_texture_asset_name"].GetString();
	m_green_channel_mapped_texture_asset_name = doc["green_channel_mapped_texture_asset_name"].GetString();
	m_blue_channel_mapped_texture_asset_name = doc["blue_channel_mapped_texture_asset_name"].GetString();
	m_alpha_channel_mapped_texture_asset_name = doc["alpha_channel_mapped_texture_asset_name"].GetString();
	check_referenced_textures_are_valid();
}

void terrain::shutdown()
{
	using namespace gl;
	m_heights.clear();
	const size_t n_tiles = m_renderable_tiles.size();
	std::vector<GLuint> vao_ids(n_tiles);
	std::vector<GLuint> vertex_buffer_ids(n_tiles);
	std::vector<GLuint> index_buffer_ids(n_tiles);
	for (size_t i = 0; i < n_tiles; ++i)
	{
		vao_ids[i] = m_renderable_tiles[i].vertex_array_object_id;
		vertex_buffer_ids[i] = m_renderable_tiles[i].vertex_buffer_id;
		index_buffer_ids[i] = m_renderable_tiles[i].index_buffer_id;
	}
	glDeleteVertexArrays(n_tiles, vao_ids.data());
	glDeleteBuffers(n_tiles, vertex_buffer_ids.data());
	glDeleteBuffers(n_tiles, index_buffer_ids.data());
	vao_ids.clear();
	vertex_buffer_ids.clear();
	index_buffer_ids.clear();
	m_renderable_tiles.clear();
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

float terrain::get_height_at(float x_world_space, float z_world_space) const
{
	// treat x+ as west to east
	// treat z+ as north to south. this is how it would appear in the tiff file
	const float tiff_length_as_float = static_cast<float>(m_geo_tiff_height_info.length);
	const float tiff_width_as_float = static_cast<float>(m_geo_tiff_height_info.width);

	const float far_west = -(tiff_width_as_float * 0.5f) * m_geo_tiff_height_info.meters_per_pixel_x;
	const float far_north = -(tiff_length_as_float * 0.5f) * m_geo_tiff_height_info.meters_per_pixel_z;
	const float far_south = (tiff_length_as_float * 0.5f) * m_geo_tiff_height_info.meters_per_pixel_z;
	const float far_east = (tiff_width_as_float * 0.5f) * m_geo_tiff_height_info.meters_per_pixel_x;

	const bool too_far_north = z_world_space <= far_north;
	const bool too_far_south = z_world_space >= far_south;
	const bool too_far_west = x_world_space <= far_west;
	const bool too_far_east = x_world_space >= far_east;

	if (too_far_north || too_far_south || too_far_west || too_far_east) return 0.0f;

	// will otherwise bilerp so will 
	const float position_in_tiff_x = x_world_space - far_west;
	const float position_in_tiff_z = z_world_space - far_north;
	const float x_weight = position_in_tiff_x - std::floorf(position_in_tiff_x);
	const float z_weight = position_in_tiff_z - std::floorf(position_in_tiff_z);

	const uint16 x_tiff_pixel_west = static_cast<uint16>(std::floorf(position_in_tiff_x));
	const uint16 y_tiff_pixel_north = static_cast<uint16>(std::floorf(position_in_tiff_z));
	const uint16 x_tiff_pixel_east = x_tiff_pixel_west + 1;
	const uint16 y_tiff_pixel_south = y_tiff_pixel_north + 1;

	const float x_north = glm::lerp(get_tiff_height_at(x_tiff_pixel_west, y_tiff_pixel_north), get_tiff_height_at(x_tiff_pixel_east, y_tiff_pixel_north), x_weight);
	const float x_south = glm::lerp(get_tiff_height_at(x_tiff_pixel_west, y_tiff_pixel_south), get_tiff_height_at(x_tiff_pixel_east, y_tiff_pixel_south), x_weight);
	return glm::lerp(x_north, x_south, z_weight);
}

const std::vector<renderable_tile_area>& terrain::get_renderable_tiles() const
{
	return m_renderable_tiles;
}

gl::GLuint terrain::get_splat_map_texture_id() const
{
	return get_texture_id(m_splat_map_asset_name);
}

gl::GLuint terrain::get_red_channel_mapped_texture_texture_id() const
{
	return get_texture_id(m_red_channel_mapped_texture_asset_name);
}

gl::GLuint terrain::get_green_channel_mapped_texture_texture_id() const
{
	return get_texture_id(m_green_channel_mapped_texture_asset_name);
}

gl::GLuint terrain::get_blue_channel_mapped_texture_texture_id() const
{
	return get_texture_id(m_blue_channel_mapped_texture_asset_name);
}

gl::GLuint terrain::get_alpha_channel_mapped_texture_texture_id() const
{
	return get_texture_id(m_alpha_channel_mapped_texture_asset_name);
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

uint64 terrain::get_height_index(uint16 x_tiff_pixels, uint16 y_tiff_pixels) const
{
	uint64 results = (m_geo_tiff_height_info.width * y_tiff_pixels) + x_tiff_pixels;
	assert(results >= y_tiff_pixels && results>= x_tiff_pixels);
	return results;
}

void terrain::generate_open_gl_buffers()
{
	using namespace std;
	using namespace vertex_types;
	using namespace gl;

	// TODO: Use ROAM to make the tri count smaller. think quad tree sub devision until the leaf note has no delta in height, actually. better to do it on if (delta > threshold) { devide();}
	// Since the tileing keeps coming out wrong and it's probably the index buffer, the different approach "might fix it.".

	// TODO: the length isn't uniform per pixel. we'll need to update to account for that.

	const uint32 tiff_width = m_geo_tiff_height_info.width;
	const uint32 tiff_length = m_geo_tiff_height_info.length;

	uint32 mutable_tile_width_px = 0, mutable_tile_length_px = 0;
	calculate_tile_dimensions_needed_for_uint16_index_buffer(tiff_width, tiff_length, mutable_tile_width_px, mutable_tile_length_px);
	const uint32 tile_width_px = mutable_tile_width_px;
	const uint32 tile_length_px = mutable_tile_length_px;

	assert(tiff_width % tile_width_px == 0);
	assert(tiff_length % tile_length_px == 0);
	const size_t tiles_west_to_east = tiff_width / tile_width_px;
	const size_t tiles_north_to_south = tiff_length / tile_length_px;
	const size_t n_tiles_to_make = tiles_west_to_east * tiles_north_to_south;

	GLuint* vao_ids = new GLuint[n_tiles_to_make];
	glGenVertexArrays(n_tiles_to_make, vao_ids);
	GLuint* vertex_buffer_ids = new GLuint[n_tiles_to_make];
	glGenBuffers(n_tiles_to_make, vertex_buffer_ids);
	GLuint* index_buffer_ids = new GLuint[n_tiles_to_make];
	glGenBuffers(n_tiles_to_make, index_buffer_ids);

	m_renderable_tiles.resize(n_tiles_to_make);

	vector<terrain_vertex> vertex_buffer_data; // declared outside the loop to avoid thrashing the stack.
	vector<uint16_t> index_buffer_data;

	for (uint32 i = 0; i < tiles_north_to_south; ++i)
	{
		const bool need_overlap_with_south_cell = i < tiles_north_to_south - 1;
		for (uint32 j = 0; j < tiles_west_to_east; ++j)
		{
			const uint32 tile_index = j + (tiles_west_to_east * i);
			renderable_tile_area& to_set = m_renderable_tiles[tile_index];
			const bool need_overlap_with_east_cell = j < tiles_west_to_east - 1;

			// determine the north, south, west, east px in the tiff bounds.
			const uint32 north_tiff_px = tile_length_px * i;
			const uint32 west_tiff_px = tile_width_px * j;
			const uint32 south_tiff_px = tile_length_px * (i + 1) + (need_overlap_with_south_cell ? 1 : 0);
			const uint32 east_tiff_px = tile_width_px * (j + 1) + (need_overlap_with_east_cell ? 1 : 0);

			generate_tile_vertex_and_index_buffer_data(north_tiff_px, south_tiff_px, west_tiff_px, east_tiff_px, vertex_buffer_data, index_buffer_data);
			set_tile_bounds(vertex_buffer_data, to_set);
			sanity_check_buffer_data(vertex_buffer_data, index_buffer_data); // debug code

			to_set.tile_index = tile_index;
			to_set.vertex_buffer_id = vertex_buffer_ids[to_set.tile_index];
			to_set.vertex_array_object_id = vao_ids[to_set.tile_index];
			to_set.index_buffer_id = index_buffer_ids[to_set.tile_index];


			setup_vertex_attrib_array(to_set.vertex_array_object_id);
			glBindBuffer(GL_ARRAY_BUFFER, to_set.vertex_buffer_id);
			glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size() * terrain_vertex_struct_size, vertex_buffer_data.data(), GL_STATIC_DRAW);
			to_set.num_indices_to_draw = index_buffer_data.size();
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, to_set.index_buffer_id);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_data.size() * sizeof(uint16_t), index_buffer_data.data(), GL_STATIC_DRAW);

			vertex_buffer_data.clear();
			index_buffer_data.clear();
		}
	}

	delete[] vao_ids;
	delete[] vertex_buffer_ids;
	delete[] index_buffer_ids;

	gl::glBindVertexArray(0);
	gl::glBindBuffer(gl::GL_ARRAY_BUFFER, 0);
}

std::vector<uint32_t> terrain::get_all_whole_denominators_sorted(uint32_t x)
{
	using namespace std;
	vector<uint32_t> results;
	if (x == 0) return results; // 0 has infinitely many divisors, return empty
	const uint32_t limit = static_cast<uint32_t>(sqrt(x));
	for (uint32_t i = 1; i <= limit; ++i)
	{
		if (x % i == 0)
		{
			results.push_back(i);
			uint32_t other = x / i;
			if (other != i) results.push_back(other);
		}
	}
	sort(begin(results), end(results));
	return results;
}

void terrain::calculate_tile_dimensions_needed_for_uint16_index_buffer(
	uint32 width_px,
	uint32 length_px,
	uint32& output_tile_width_px,
	uint32& output_tile_length_px)
{
	using namespace std;
	constexpr uint32 max_vertices_to_support = numeric_limits<uint16>::max();

	// Basic validation
	assert(width_px > 1);
	assert(length_px > 1);

	// Set to be 1x1 tile(s)
	output_tile_width_px = width_px;
	output_tile_length_px = length_px;

	// If already fits, done.
	if (width_px * length_px <= max_vertices_to_support) return;

	const vector<uint32> tile_widths = get_all_whole_denominators_sorted(width_px);
	const vector<uint32> tile_lengths = get_all_whole_denominators_sorted(length_px);

	const uint32 max_combinations = min(tile_widths.size(), tile_lengths.size());
	for (uint32 i = 0; i < max_combinations; ++i)
	{
		// remember we want the overlap on the cell to the east and south.
		const uint32 width_to_check = tile_widths[i] + 1;
		const uint32 length_to_check = tile_lengths[i] + 1;
		if ((width_to_check * length_to_check) + 4 <= max_vertices_to_support)
		{
			output_tile_width_px = tile_widths[i];
			output_tile_length_px = tile_lengths[i];
		}
		else
		{
			return; // the values are sorted, we've stepped up to the limit.
		}
	}
}

void terrain::generate_tile_vertex_and_index_buffer_data(uint32 tiff_north_px, uint32 tiff_south_px, uint32 tiff_west_px, uint32 tiff_east_px, std::vector<vertex_types::terrain_vertex>& out_vertex_buffer, std::vector<uint16_t>& out_index_buffer) const
{
	// this would be static but want to read from the tiff
	const uint32 tile_width = tiff_east_px - tiff_west_px;
	const uint32 tile_length = tiff_south_px - tiff_north_px;
	const size_t tile_area = tile_width * tile_length;
	assert(tile_area <= std::numeric_limits<uint16_t>::max());
	out_vertex_buffer.resize(tile_area);
	for (uint32 i = 0; i < tile_length; ++i)
	{
		for (uint32 j = 0; j < tile_width; ++j)
		{
			const int vertex_buffer_index_offset = (i * tile_width) + j;
			assert(vertex_buffer_index_offset < out_vertex_buffer.size());

			const size_t current_px_j = (tiff_west_px + j);
			const size_t current_px_i = (tiff_north_px + i);
			out_vertex_buffer[vertex_buffer_index_offset] = get_vertex_for_tiff_pixel(current_px_j, current_px_i); 
		}
	}
	out_index_buffer.clear();

	const size_t quad_count = (tile_width - 1) * (tile_length - 1);
	out_index_buffer.reserve(quad_count * 6);

	for (uint32 y = 0; y < tile_length - 1; ++y)
	{
		for (uint32 x = 0; x < tile_width - 1; ++x)
		{
			const uint16 i0 = y * tile_width + x;
			const uint16 i1 = y * tile_width + (x + 1);
			const uint16 i2 = (y + 1) * tile_width + x;
			const uint16 i3 = (y + 1) * tile_width + (x + 1);

			assert(i0 >= y && i0 >= x);
			assert(i1 >= y && i1 >= x);
			assert(i2 >= y && i2 >= x);
			assert(i3 >= y && i3 >= x);

			out_index_buffer.push_back(i0);
			out_index_buffer.push_back(i2);
			out_index_buffer.push_back(i1);

			out_index_buffer.push_back(i1);
			out_index_buffer.push_back(i2);
			out_index_buffer.push_back(i3);
		}
	}
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

void terrain::check_referenced_textures_are_valid()
{
	auto asset_manager = get_asset_manager().lock();
	for (auto asset_name : { m_splat_map_asset_name, m_red_channel_mapped_texture_asset_name,
		m_green_channel_mapped_texture_asset_name, m_blue_channel_mapped_texture_asset_name,m_alpha_channel_mapped_texture_asset_name})
	{
		asset_manager->get_asset_on_name(asset_name); // throws if the asset isn't present.
	}
}

void terrain::sanity_check_buffer_data(const std::vector<vertex_types::terrain_vertex>& vertex_buffer_data, const std::vector<uint16_t>& index_buffer_data)
{
	using namespace vertex_types;
	const size_t vb_size = vertex_buffer_data.size();
	assert(vb_size % 4 == 0);
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

gl::GLuint terrain::get_texture_id(std::string_view texture_asset_name) const
{
	// TODO: cache the texture ids.
	std::weak_ptr<const asset> result = get_asset_manager().lock()->get_asset_on_name(texture_asset_name);
	return std::dynamic_pointer_cast<const texture>(result.lock())->get_id();
}