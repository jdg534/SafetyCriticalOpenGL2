#include "terrain.h"

#include <filesystem>
// #include <gdal>

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

	
}

void terrain::shutdown()
{
	// freeup stuff.
}

asset_type terrain::get_type() const
{
	return asset_type::terrain;
}