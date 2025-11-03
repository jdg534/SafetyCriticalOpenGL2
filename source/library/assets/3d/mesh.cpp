#include "mesh.h"

#include <string_view>
#include <filesystem>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "../../render/vertex_types.h"

using namespace std;

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
    using namespace vertex_types;

    if (initialise_with->HasBones())
    {
        throw runtime_error("Attempted to load rigged mesh, this isn't currently supported");
    }
    
    if (!initialise_with->HasTextureCoords(0) || !initialise_with->HasPositions() || !initialise_with->HasNormals())
    {
        throw runtime_error("Only meshes with normals and texture coordinates are supported");
    }

    std::vector<vertex_3d> vertex_buffer_init_data;
    std::vector<unsigned short> index_buffer_init_data;

    vertex_buffer_init_data.resize(initialise_with->mNumVertices);
    index_buffer_init_data.resize(initialise_with->mNumFaces * 3);

    for (unsigned int i = 0; i < initialise_with->mNumVertices; ++i)
    {
        vertex_buffer_init_data[i].position.x = initialise_with->mVertices[i].x;
        vertex_buffer_init_data[i].position.y = initialise_with->mVertices[i].y;
        vertex_buffer_init_data[i].position.z = initialise_with->mVertices[i].z;

        vertex_buffer_init_data[i].normal.x = initialise_with->mNormals[i].x;
        vertex_buffer_init_data[i].normal.y = initialise_with->mNormals[i].y;
        vertex_buffer_init_data[i].normal.z = initialise_with->mNormals[i].z;

        vertex_buffer_init_data[i].texture_coordinates.x = initialise_with->mTextureCoords[0][i].x;
        vertex_buffer_init_data[i].texture_coordinates.y = initialise_with->mTextureCoords[0][i].y;
    }

    for (unsigned int i = 0; i < initialise_with->mNumFaces; ++i)
    {
        const aiFace& face = initialise_with->mFaces[i];
        if (!face.mNumIndices == 3)
        {
            throw runtime_error("Only triangular meshes are supported!");
        }
        const unsigned int write_index = i * 3;
        index_buffer_init_data[write_index + 0] = face.mIndices[0];
        index_buffer_init_data[write_index + 1] = face.mIndices[1];
        index_buffer_init_data[write_index + 2] = face.mIndices[2];
    }

    throw runtime_error("TODO: finish setting up the index and vertex buffers, once they're on the GPU no need to keep them in normal RAM");
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
