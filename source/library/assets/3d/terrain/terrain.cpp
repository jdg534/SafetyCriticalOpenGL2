#include "terrain.h"

#include "../../asset_utils.h"

#include <tiffio.h>
#include <geotiff.h>
#include <geo_normalize.h>
#include <xtiffio.h>
#include <rapidjson/document.h>

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

	uint32 width = 0, height = 0;
	TIFFGetField(tiff_file, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(tiff_file, TIFFTAG_IMAGELENGTH, &height);

	uint16 bitsPerSample = 0;
	uint16 sampleFormat = 0;
	uint16 samplesPerPixel = 0;

	TIFFGetField(tiff_file, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
	TIFFGetFieldDefaulted(tiff_file, TIFFTAG_SAMPLEFORMAT, &sampleFormat);
	TIFFGetField(tiff_file, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);

	if (samplesPerPixel != 1) { throw std::runtime_error("Expected single-band heightmap"); }
	if (bitsPerSample != 8) { throw std::runtime_error("Expected 8 bit height data"); }
	if (sampleFormat == SAMPLEFORMAT_VOID || sampleFormat == SAMPLEFORMAT_IEEEFP) { throw std::runtime_error("unsupported height data format"); }

	m_heights.resize(width * height);
	if (sampleFormat == SAMPLEFORMAT_UINT)
	{
		read_heights_uint8(m_heights, tiff_file);
	}
	else if (sampleFormat == SAMPLEFORMAT_INT)
	{
		read_heights_sint8(m_heights, tiff_file);
	}

	GTIF* geo_tif = GTIFNew(tiff_file);
	if (!geo_tif)
		throw std::runtime_error("GTIFNew failed");


	uint16 geo_pixel_scale = 0, geo_tie_points = 0;
	if (!TIFFGetField(tiff_file, TIFFTAG_GEOPIXELSCALE, &geo_pixel_scale)
		)
	{
		std::cout << "Missing geo tiff pixel scale" << std::endl;
	}
	if (!TIFFGetField(tiff_file, TIFFTAG_GEOTIEPOINTS, &geo_tie_points))
	{
		std::cout << "Missing geo tiff tie points" << std::endl;
	}

	GTIFFree(geo_tif);
	XTIFFClose(tiff_file);
	tiff_file = nullptr;

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