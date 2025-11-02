#include "mesh.h"

#include <string_view>
#include <filesystem>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

// public
/////////

mesh::mesh(const std::string& name, const std::string& path, std::weak_ptr<const asset_manager> asset_manager)
	: asset(name, path, asset_manager)
{

}

mesh::~mesh()
{
	// TODO: CODE ME!
}

void mesh::initialise()
{
    // this is a placeholder
}

void mesh::initialise_assimp_struct(const aiMesh* initialise_with)
{
	throw std::runtime_error("TODO: code mesh::initialise_assimp_struct()");
}

void mesh::shutdown()
{
	// TODO: CODE ME!
}

asset_type mesh::get_type() const
{
	return asset_type::mesh;
}

// private
//////////
