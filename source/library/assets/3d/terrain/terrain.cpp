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
	TIFFGetField(tiff_file, TIFFTAG_SAMPLEFORMAT, &sampleFormat);
	TIFFGetField(tiff_file, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);

	if (samplesPerPixel != 1)
		throw std::runtime_error("Expected single-band heightmap");

	if (bitsPerSample != 32 || sampleFormat != SAMPLEFORMAT_IEEEFP) // LLM generated code... Update it to support 8 bit tiff files.
		throw std::runtime_error("Expected Float32 height data");

	std::vector<float> heights(width * height);

	for (uint32 row = 0; row < height; ++row)
	{
		float* rowPtr = heights.data() + row * width;

		if (TIFFReadScanline(tiff_file, rowPtr, row) != 1) throw std::runtime_error("TIFFReadScanline failed");
	}



	XTIFFClose(tiff_file);
	tiff_file = nullptr;

}

void terrain::shutdown()
{
	// freeup stuff.
}

asset_type terrain::get_type() const
{
	return asset_type::terrain;
}