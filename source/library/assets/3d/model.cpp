#include "model.h"

#include <string_view>
#include <filesystem>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

// public
/////////

model::model(const std::string& name, const std::string& path, std::weak_ptr<const asset_manager> asset_manager)
	: asset(name, path, asset_manager)
{

}

model::~model()
{
	// TODO: CODE ME!
}

void model::initialise()
{
    Assimp::Importer importer;

    unsigned int flags = aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_GenSmoothNormals |
        aiProcess_PreTransformVertices |
        aiProcess_ImproveCacheLocality |
        aiProcess_RemoveRedundantMaterials |
        aiProcess_MakeLeftHanded; // see if the texutre coods come out right

    const aiScene* scene = importer.ReadFile(get_path().data(), flags);
    if (!scene || !scene->mRootNode) {
        throw std::exception(importer.GetErrorString());
    }

    initialise_materials(scene->mNumMaterials, *scene->mMaterials);
    initialise_meshes(scene);
}



void model::shutdown()
{
	// TODO: CODE ME!
}

asset_type model::get_type() const
{
	return asset_type::model;
}

// private
//////////

void model::initialise_materials(unsigned int num_materials, const aiMaterial* materials)
{
    m_materials.resize(num_materials);
    for (unsigned int i = 0; i < num_materials; ++i)
    {
        const aiMaterial* current_material = materials + i;
        aiString name;
        if (AI_SUCCESS != current_material->Get(AI_MATKEY_NAME, name))
        {
            throw std::runtime_error("Could not detemine material name");
        }
        m_materials[i] = std::make_shared<material>(name.C_Str(), get_path().data(), get_asset_manager());
        m_materials[i]->initialise_assimp_struct(current_material);
    }
}

void model::initialise_meshes(const aiScene* scene)
{
    throw std::runtime_error("TODO: code model::initialise_meshes");
}
