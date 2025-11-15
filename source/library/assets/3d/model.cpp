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
    if (!scene || !scene->mRootNode) { throw std::exception(importer.GetErrorString()); }

    initialise_materials(scene->mNumMaterials, *scene->mMaterials);
    initialise_meshes(scene);
}

void model::shutdown()
{
    for (auto mesh : m_meshs) { mesh->shutdown(); }
    for (auto material : m_materials) { material->shutdown(); }
    m_meshs.clear();
    m_materials.clear();
}

asset_type model::get_type() const
{
	return asset_type::model;
}

const std::vector<std::shared_ptr<material>>& model::get_materials() const
{
    return m_materials;
}

const std::vector<std::shared_ptr<mesh>>& model::get_meshs() const
{
    return m_meshs;
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
    const unsigned int num_meshes = scene->mNumMeshes;
    m_meshs.resize(num_meshes);
    for (unsigned int i = 0; i < num_meshes; ++i)
    {
        const aiMesh* current_mesh = *scene->mMeshes + i;
        m_meshs[i] = std::make_shared<mesh>(current_mesh->mName.C_Str(), get_path().data(), get_asset_manager());
        m_meshs[i]->initialise_assimp_struct(current_mesh);
    }
}
