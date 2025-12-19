#include "terrain.h"

#include "../../asset_utils.h"

#include "../../../render/vertex_types.h"

#include <tiffio.h>
#include <geotiff.h>
#include <geo_normalize.h>
#include <xtiffio.h>
#include <rapidjson/document.h>

#include <glm/glm.hpp>

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

	TIFFGetField(tiff_file, TIFFTAG_IMAGEWIDTH, &m_tiff_width);
	TIFFGetField(tiff_file, TIFFTAG_IMAGELENGTH, &m_tiff_length);

	uint16 bitsPerSample = 0;
	uint16 sampleFormat = 0;
	uint16 samplesPerPixel = 0;

	TIFFGetField(tiff_file, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
	TIFFGetFieldDefaulted(tiff_file, TIFFTAG_SAMPLEFORMAT, &sampleFormat);
	TIFFGetField(tiff_file, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);

	if (samplesPerPixel != 1) { throw std::runtime_error("Expected single-band heightmap"); }
	if (bitsPerSample != 8) { throw std::runtime_error("Expected 8 bit height data"); }
	if (sampleFormat == SAMPLEFORMAT_VOID || sampleFormat == SAMPLEFORMAT_IEEEFP) { throw std::runtime_error("unsupported height data format"); }

	if (TIFFIsTiled(tiff_file))
	{
		throw exception("Titled tiff file.");
	}

	double* pixelScale = nullptr;
	double* tiePoints = nullptr;
	if (!TIFFGetField(tiff_file, TIFFTAG_GEOPIXELSCALE, &pixelScale))
	{
		std::cout << "Missing GeoPixelScale\n";
	}

	if (!TIFFGetField(tiff_file, TIFFTAG_GEOTIEPOINTS, &tiePoints))
	{
		std::cout << "Missing GeoTiePoints\n";
	}
	
	/*
	glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
	double* modelTransform = nullptr; // add this later if needed.
	if (TIFFGetField(tiff_file, TIFFTAG_MODELTRANSFORMATIONTAG, &modelTransform)) // if set will be a 4x4 matrix
	{
		scale.x = static_cast<float>(modelTransform[0]);
		scale.y = static_cast<float>(modelTransform[5]);
		scale.z = static_cast<float>(modelTransform[10]);
	}
	*/

	m_heights.resize(m_tiff_width * m_tiff_length);
	if (sampleFormat == SAMPLEFORMAT_UINT)
	{
		read_heights_uint8(m_heights, tiff_file);
	}
	else if (sampleFormat == SAMPLEFORMAT_INT)
	{
		read_heights_sint8(m_heights, tiff_file);
	}

	// flip_rows(m_heights, m_tiff_width, m_tiff_length); // not needed, uncomment if we need to flip

	GTIFFree(geo_tiff);
	XTIFFClose(tiff_file);
	tiff_file = nullptr;

	generate_open_gl_buffers();
}

void terrain::shutdown()
{
	// freeup stuff.
	m_heights.clear();
}

asset_type terrain::get_type() const
{
	return asset_type::terrain;
}

uint16 terrain::get_tiff_width() const
{
	return m_tiff_width;
}

uint16 terrain::get_tiff_length() const
{
	return m_tiff_length;
}

float terrain::get_tiff_height_at(uint16 x, uint16 y) const
{
	const size_t index = get_height_index(x, y);
	assert(index < m_heights.size());
	return m_heights[index];
}

// private
//////////

void terrain::read_heights_uint8(std::vector<float>& output_buffer, TIFF* tiff_file)
{
	uint32 width = 0, height = 0;
	TIFFGetField(tiff_file, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(tiff_file, TIFFTAG_IMAGELENGTH, &height);

	std::vector<uint8_t> scanline(width);
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

	std::vector<uint8_t> scanline(width);
	constexpr float INV_128 = 1.0f / 128.0f;
	for (uint32 row = 0; row < height; ++row)
	{
		if (TIFFReadScanline(tiff_file, scanline.data(), row) != 1) throw std::runtime_error("TIFFReadScanline failed");
		float* dst = output_buffer.data() + row * width;
		for (uint32 col = 0; col < width; ++col)
		{
			dst[col] = static_cast<float>(scanline[col]) * INV_128;
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
	return (m_tiff_width * y) + x;
}

void terrain::generate_open_gl_buffers()
{
	using namespace vertex_types;
	
	// TODO later if there's free time. use ROAM to make the 
	std::vector<vertex_3d> vertex_buffer_data(m_heights.size());

	// Note we're using uint32 indices, for higher range.

	const float far_west = -static_cast<float>(m_tiff_width / 2) * m_tiff_meters_per_pixel;
	const float far_north = -static_cast<float>(m_tiff_length / 2) * m_tiff_meters_per_pixel;


	std::vector<uint32_t> index_buffer_data(m_heights.size() * 6);
	for (size_t i = 0; i < m_tiff_length; ++i)
	{
		for (size_t j = 0; j < m_tiff_width; ++j)
		{
			const int vertex_buffer_index_offset = 4 * i * j;
			const int index_buffer_offset = 6 * i * j;

			const size_t left_px = j - 1;
			const size_t above_px = i - 1;
			const size_t right_px = j + 1;
			const size_t below_px = i + 1;
			const bool got_left_px = left_px >= 0;
			const bool got_above_px = above_px >= 0;
			const bool got_right_px = right_px < m_tiff_width;
			const bool got_below_px = below_px < m_tiff_width;

			const float current_px_height = get_tiff_height_at(j, i);
			const float above_px_height = got_above_px ? get_tiff_height_at(j, above_px) : 0.0f;
			const float left_px_height = got_left_px ? get_tiff_height_at(left_px, i) : 0.0f;
			const float right_px_height = got_right_px ? get_tiff_height_at(right_px, i) : 0.0f;
			const float below_px_height = got_below_px ? get_tiff_height_at(j, below_px) : 0.0f;

			vertex_buffer_data[vertex_buffer_index_offset].position.x = far_west + (j * m_tiff_meters_per_pixel);
			vertex_buffer_data[vertex_buffer_index_offset].position.y = current_px_height;
			vertex_buffer_data[vertex_buffer_index_offset].position.z = far_north + (i * m_tiff_meters_per_pixel);
			vertex_buffer_data[vertex_buffer_index_offset].texture_coordinates.x = (sinf(static_cast<float>(j)) + 1.0f) / 2.0f;
			vertex_buffer_data[vertex_buffer_index_offset].texture_coordinates.y = (sinf(static_cast<float>(i)) + 1.0f) / 2.0f;

			const glm::vec3 dx = glm::vec3(2.0f * m_tiff_meters_per_pixel, right_px_height - left_px_height, 0.0f);
			const glm::vec3 dy = glm::vec3(0.0f, above_px_height - below_px_height, 2.0f * m_tiff_meters_per_pixel);
			vertex_buffer_data[vertex_buffer_index_offset].normal = glm::normalize(glm::cross(dy, dx));
		}
	}
	// now the index buffer!
}
