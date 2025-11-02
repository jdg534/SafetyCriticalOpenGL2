#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include <map>
#include <memory>
#include <string>
#include <string_view>

#include <assimp/material.h>
#include <glm/glm.hpp>

#include "../asset.h"
#include "../texture.decl.h"


enum class texture_purpose : std::uint8_t
{
    diffuse_map,
    specular_map,
    ambient_map,
    normal_map,
    height_map,
    emissive_map,
    opacity_map,
    metalness_map,
};

class material : public asset
{
public:

    material() = delete;
    material(const std::string& name, const std::string& path, std::weak_ptr<const asset_manager> asset_manager);
	virtual ~material();

	void initialise() override;
    void initialise_assimp_struct(const aiMaterial* assimp_material);
	void shutdown() override;
	asset_type get_type() const override;

private:

    glm::vec4 m_diffuse_colour { 1.0f, 1.0f, 1.0f, 1.0f };
    glm::vec4 m_ambient_colour  { 0.0f, 0.0f, 0.0f, 1.0f };
    glm::vec4 m_specular_colour { 0.0f, 0.0f, 0.0f, 1.0f };
    float m_shininess = 0.0f;

    std::unordered_map<texture_purpose, std::weak_ptr<texture>> m_textures;
};

#endif // _MATERIAL_H_
