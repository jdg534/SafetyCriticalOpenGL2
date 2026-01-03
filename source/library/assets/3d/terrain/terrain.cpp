#include "terrain.h"

#include "../../asset_utils.h"

#include "../../../render/vertex_types.h"
#include "../../asset_manager.h"
#include "../../texture.h"

#include <tiffio.h>
#include <geotiff.h>
#include <geo_normalize.h>
#include <xtiffio.h>
#include <rapidjson/document.h>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL // done for lerp, otherwise remove this.
#include <glm/gtx/compatibility.hpp>

#include <cmath>
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

	// --- Pixel scale ---
	// skipping for now. need to code the number range. defaulting to units being in meters.
	/*
	unsigned short num_pixel_scale_count = 0;
	double* pixel_scale = nullptr;
	if (TIFFGetField(tiff_file, TIFFTAG_GEOPIXELSCALE,&num_pixel_scale_count, &pixel_scale))
	{
		assert(pixel_scale != nullptr);
		m_geo_tiff_height_info.meters_per_pixel_x = pixel_scale[0];
		m_geo_tiff_height_info.meters_per_pixel_y = pixel_scale[1];
	}
	*/

	// --- Vertical units ---
	short vertical_units = 0;
	if (GTIFKeyGet(geo_tiff, VerticalUnitsGeoKey, &vertical_units, 0, 1))
	{
		// EPSG codes
		// 9001 = meters
		// 9002 = feet
		m_geo_tiff_height_info.vertical_units_are_meters = (vertical_units == 9001);
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

	glDeleteVertexArrays(1, &m_vertex_array_object_id);
	GLuint buffer_ids[2] = { m_vertex_buffer_id, m_index_buffer_id };
	glDeleteBuffers(2, buffer_ids);
	m_vertex_array_object_id = m_vertex_buffer_id = m_index_buffer_id = m_num_indices_to_draw = 0;
}

asset_type terrain::get_type() const
{
	return asset_type::terrain;
}

const geo_tiff_height_info& terrain::get_height_info() const
{
	return m_geo_tiff_height_info;
}

float terrain::get_tiff_height_at(uint16 x, uint16 y) const
{
	const size_t index = get_height_index(x, y);
	assert(index < m_heights.size());
	return m_heights[index];
}

float terrain::get_height_range_value_at(uint16 x, uint16 y) const
{
	if (m_geo_tiff_height_info.use_raw_height_value) // if this flag is set, then values aren't normalised and aren't in range 0.0f to 1.0f (not meant to be lerped)
	{
		return get_tiff_height_at(x, y);
	}
	// because the heights are converted to 0.0 to 1.0f, we can just lerp
	return glm::lerp(m_geo_tiff_height_info.height_min_meters, m_geo_tiff_height_info.height_max_meters, get_tiff_height_at(x, y));
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

	const uint16 x_cell = static_cast<uint16>(std::floorf(position_in_tiff_x));
	const uint16 z_cell = static_cast<uint16>(std::floorf(position_in_tiff_z));
	const uint16 x_cell_east = x_cell + 1;
	const uint16 z_cell_south = z_cell + 1;

	const float x_north = glm::lerp(get_height_range_value_at(x_cell, z_cell), get_height_range_value_at(x_cell_east, z_cell), x_weight);
	const float x_south = glm::lerp(get_height_range_value_at(x_cell, z_cell_south), get_height_range_value_at(x_cell_east, z_cell_south), x_weight);
	return glm::lerp(x_north, x_south, z_weight);
}

gl::GLuint terrain::get_vertex_array_object_id() const
{
	return m_vertex_array_object_id;
}

gl::GLuint terrain::get_vertex_buffer_id() const
{
	return m_vertex_buffer_id;
}

gl::GLuint terrain::get_index_buffer_id() const
{
	return m_index_buffer_id;
}

gl::GLuint terrain::get_num_indices_to_draw() const
{
	return m_num_indices_to_draw;
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

size_t terrain::get_height_index(uint16 x, uint16 y) const
{
	return (m_geo_tiff_height_info.width * y) + x;
}

void terrain::generate_open_gl_buffers()
{
	using namespace vertex_types;
	
	// TODO later if there's free time. use ROAM to make the tri count smaller. think quad tree sub devision until the leaf note has no delta in height.
	std::vector<vertex_3d> vertex_buffer_data(m_heights.size());

	// Note we're using uint32 indices, for higher range.
	
	const uint32 tiff_width = m_geo_tiff_height_info.width;
	const uint32 tiff_length = m_geo_tiff_height_info.length;

	const float tiff_length_as_float = static_cast<float>(tiff_length);
	const float tiff_width_as_float = static_cast<float>(tiff_width);

	const float far_west = -(tiff_width_as_float * 0.5f) * m_geo_tiff_height_info.meters_per_pixel_x;
	const float far_north = -(tiff_length_as_float * 0.5f) * m_geo_tiff_height_info.meters_per_pixel_z;

	for (size_t i = 0; i < m_geo_tiff_height_info.length; ++i)
	{
		for (size_t j = 0; j < m_geo_tiff_height_info.width; ++j)
		{
			const int vertex_buffer_index_offset = (i * m_geo_tiff_height_info.width) + j;

			const size_t left_px = j - 1;
			const size_t above_px = i - 1;
			const size_t right_px = j + 1;
			const size_t below_px = i + 1;
			const bool got_left_px = j > 0;
			const bool got_above_px = i > 0;
			const bool got_right_px = right_px < m_geo_tiff_height_info.width;
			const bool got_below_px = below_px < m_geo_tiff_height_info.length;

			const float current_px_height = get_height_range_value_at(j, i);
			const float above_px_height = got_above_px ? get_height_range_value_at(j, above_px) : current_px_height;
			const float left_px_height = got_left_px ? get_height_range_value_at(left_px, i) : current_px_height;
			const float right_px_height = got_right_px ? get_height_range_value_at(right_px, i) : current_px_height;
			const float below_px_height = got_below_px ? get_height_range_value_at(j, below_px) : current_px_height;

			vertex_buffer_data[vertex_buffer_index_offset].position.x = far_west + (j * m_geo_tiff_height_info.meters_per_pixel_x);
			vertex_buffer_data[vertex_buffer_index_offset].position.y = current_px_height;
			vertex_buffer_data[vertex_buffer_index_offset].position.z = far_north + (i * m_geo_tiff_height_info.meters_per_pixel_z);
			vertex_buffer_data[vertex_buffer_index_offset].texture_coordinates.x = static_cast<float>(j) / tiff_width_as_float;
			vertex_buffer_data[vertex_buffer_index_offset].texture_coordinates.y = 1.0f - (static_cast<float>(i) / tiff_length_as_float);

			const glm::vec3 dx = glm::vec3(2.0f * m_geo_tiff_height_info.meters_per_pixel_x, right_px_height - left_px_height, 0.0f);
			const glm::vec3 dy = glm::vec3(0.0f, above_px_height - below_px_height, 2.0f * m_geo_tiff_height_info.meters_per_pixel_z);
			vertex_buffer_data[vertex_buffer_index_offset].normal = glm::normalize(glm::cross(dy, dx));
		}
	}

	std::vector<uint32_t> index_buffer_data;
	index_buffer_data.reserve((m_geo_tiff_height_info.width - 1) * (m_geo_tiff_height_info.length - 1) * 6);
	for (int y = 0; y < m_geo_tiff_height_info.length - 1; ++y)
	{
		for (int x = 0; x < m_geo_tiff_height_info.width - 1; ++x)
		{
			uint32_t i0 = y * tiff_width + x;
			uint32_t i1 = y * tiff_width + (x + 1);
			uint32_t i2 = (y + 1) * tiff_width + x;
			uint32_t i3 = (y + 1) * tiff_width + (x + 1);
			// tri 1: i0, i2, i1
			index_buffer_data.push_back(i0); index_buffer_data.push_back(i2); index_buffer_data.push_back(i1);
			// tri 2: i1, i2, i3
			index_buffer_data.push_back(i1); index_buffer_data.push_back(i2); index_buffer_data.push_back(i3);
		}
	}

	gl::glGenVertexArrays(1, &m_vertex_array_object_id);
	gl::glBindVertexArray(m_vertex_array_object_id);

	gl::glGenBuffers(1, &m_vertex_buffer_id);
	gl::glBindBuffer(gl::GL_ARRAY_BUFFER, m_vertex_buffer_id);
	gl::glBufferData(gl::GL_ARRAY_BUFFER, vertex_buffer_data.size() * vertex3d_struct_size, vertex_buffer_data.data(), gl::GL_STATIC_DRAW);

	gl::glGenBuffers(1, &m_index_buffer_id);
	gl::glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, m_index_buffer_id);
	gl::glBufferData(gl::GL_ELEMENT_ARRAY_BUFFER, index_buffer_data.size() * sizeof(uint32_t), index_buffer_data.data(), gl::GL_STATIC_DRAW);

	// attribute layout: 0 = position, 1 = texture_coordinates ,2 = normal, 3 = splat_map_texture_coordinates
	gl::glEnableVertexAttribArray(0);
	gl::glVertexAttribPointer(0, 3, gl::GL_FLOAT, gl::GL_FALSE, vertex3d_struct_size, (void*)offsetof(vertex_3d, position));
	gl::glEnableVertexAttribArray(1);
	gl::glVertexAttribPointer(1, 2, gl::GL_FLOAT, gl::GL_FALSE, vertex3d_struct_size, (void*)offsetof(vertex_3d, texture_coordinates));
	gl::glEnableVertexAttribArray(2);
	gl::glVertexAttribPointer(2, 3, gl::GL_FLOAT, gl::GL_FALSE, vertex3d_struct_size, (void*)offsetof(vertex_3d, normal));

	gl::glBindVertexArray(0);
	m_num_indices_to_draw = index_buffer_data.size();

	vertex_buffer_data.clear();
	index_buffer_data.clear();
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

gl::GLuint terrain::get_texture_id(std::string_view texture_asset_name) const
{
	// TODO: cache the texture ids.
	std::weak_ptr<const asset> result = get_asset_manager().lock()->get_asset_on_name(texture_asset_name);
	return std::dynamic_pointer_cast<const texture>(result.lock())->get_id();
}