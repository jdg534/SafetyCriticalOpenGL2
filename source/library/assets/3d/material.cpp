#include "material.h"

#include <string_view>
#include <filesystem>
#include <exception>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "../asset_manager.h"
#include "../asset_utils.h"
#include "../texture.h"

using namespace std;


inline texture_purpose texture_purpose_from_assimp_enum(aiTextureType texture_type)
{
    switch (texture_type)
    {
        case aiTextureType_DIFFUSE: return texture_purpose::diffuse_map;
        case aiTextureType_SPECULAR: return texture_purpose::specular_map;
        case aiTextureType_AMBIENT: return texture_purpose::ambient_map;
        case aiTextureType_NORMALS: return texture_purpose::normal_map;
        case aiTextureType_HEIGHT: return texture_purpose::height_map;
        case aiTextureType_EMISSIVE: return texture_purpose::emissive_map;
        case aiTextureType_OPACITY: return texture_purpose::opacity_map;
        case aiTextureType_METALNESS: return texture_purpose::metalness_map;
        default: break;
    }
    throw std::runtime_error("Unrecognised texture purpose found");
    return texture_purpose::diffuse_map;
}


material::material(const string& name, const string& path, weak_ptr<const asset_manager> asset_manager)
	: asset(name, path, asset_manager)
{

}

material::~material()
{
	// placeholder, this material objects don't put stuff on the head, so nothing to do.
}

void material::initialise()
{
	// This is a placeholder, this method needed to be overriden
	// initialise_assimp_struct() should be used instead
}

void material::initialise_assimp_struct(const aiMaterial* assimp_material)
{
    // colors
    aiColor4D current_colour;
    if (AI_SUCCESS == assimp_material->Get(AI_MATKEY_COLOR_DIFFUSE, current_colour))
    {
        m_diffuse_colour.r = current_colour.r;
        m_diffuse_colour.g = current_colour.g;
        m_diffuse_colour.b = current_colour.b;
        m_diffuse_colour.a = current_colour.a;
    }
    if (AI_SUCCESS == assimp_material->Get(AI_MATKEY_COLOR_AMBIENT, current_colour))
    {
        m_ambient_colour.r = current_colour.r;
        m_ambient_colour.g = current_colour.g;
        m_ambient_colour.b = current_colour.b;
        m_ambient_colour.a = current_colour.a;
    }
    if (AI_SUCCESS == assimp_material->Get(AI_MATKEY_COLOR_SPECULAR, current_colour))
    {
        m_specular_colour.r = current_colour.r;
        m_specular_colour.g = current_colour.g;
        m_specular_colour.b = current_colour.b;
        m_specular_colour.a = current_colour.a;
    }
    assimp_material->Get(AI_MATKEY_SHININESS, m_shininess);
    m_textures.clear();
    shared_ptr<const asset_manager> l_asset_manager = get_asset_manager().lock();
    for (auto assimp_texture_purpose : { aiTextureType_DIFFUSE, aiTextureType_SPECULAR, aiTextureType_AMBIENT, aiTextureType_NORMALS,
        aiTextureType_HEIGHT, aiTextureType_EMISSIVE, aiTextureType_OPACITY, aiTextureType_METALNESS })
    {
        unsigned int relevent_texture_count = assimp_material->GetTextureCount(assimp_texture_purpose);
        if (relevent_texture_count > 0)
        {
            aiString path;
            for (unsigned int i = 0; i < relevent_texture_count; ++i)
            {
                assimp_material->GetTexture(assimp_texture_purpose, i, &path);
                texture_purpose purpose = texture_purpose_from_assimp_enum(assimp_texture_purpose);
                string resolved_path = asset_utils::resolve_file_path(path.C_Str(), asset_utils::get_directory_path(get_path())); // ensure an absolute path.
                const filesystem::path working_dir = filesystem::current_path();
                resolved_path = resolved_path.substr(working_dir.string().size() + 1); // relative to the working dir.
                replace(resolved_path.begin(), resolved_path.end(), '\\', '/');
                // note the asset list needs to declare the texture before the model
                m_textures[purpose] = dynamic_pointer_cast<const texture>(l_asset_manager->get_asset_on_path(resolved_path).lock());
            }
        }
    }
}

void material::shutdown()
{
	// placeholder, we don't put anything on heap. The asset manager will remove the textures.
}

asset_type material::get_type() const
{
	return asset_type::material;
}

const glm::vec4& material::get_diffuse_colour() const
{
    return m_diffuse_colour;
}

const glm::vec4& material::get_ambient_colour() const
{
    return m_ambient_colour;
}

const glm::vec4& material::get_specular_colour() const
{
    return m_specular_colour;
}

float material::get_shininess() const
{
    return m_shininess;
}

const std::unordered_map<texture_purpose, std::weak_ptr<const texture>>& material::get_textures() const
{
    return m_textures;
}