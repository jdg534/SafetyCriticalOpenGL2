#ifndef _MODEL_H_
#define _MODEL_H_

#include <memory>
#include <string>
#include <string_view>

#include <assimp/material.h>
#include <assimp/scene.h>

#include "../asset.h"
#include "material.h" // refactor the fwd decl
#include "mesh.h" // refactor the fwd decl

class model : public asset
{
public:

	model() = delete;
	model(const std::string& name, const std::string& path, std::weak_ptr<const asset_manager> asset_manager);
	virtual ~model();

	void initialise() override;
	void shutdown() override;
	asset_type get_type() const override;

private:

	void initialise_materials(unsigned int num_materials, const aiMaterial* materials);
	void initialise_meshes(const aiScene* scene);

	std::vector<std::shared_ptr<material>> m_materials;
	std::vector<std::shared_ptr<mesh>> m_meshs;
};

#endif // _MODEL_H_
