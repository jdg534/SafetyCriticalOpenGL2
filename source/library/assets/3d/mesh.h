#ifndef _MESH_H_
#define _MESH_H_

#include <memory>
#include <string>
#include <string_view>

#include <assimp/mesh.h>

#include "../asset.h"
#include "material.h"

class mesh : public asset
{
public:

	mesh() = delete;
	mesh(const std::string& name, const std::string& path, std::weak_ptr<const asset_manager> asset_manager);
	virtual ~mesh();

	void initialise() override;
	void initialise_assimp_struct(const aiMesh* initialise_with);
	void shutdown() override;
	asset_type get_type() const override;

private:

};

#endif // _MESH_H_
