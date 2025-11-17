#ifndef _MESH_H_
#define _MESH_H_

#include <memory>
#include <string>
#include <string_view>

#include <assimp/mesh.h>
#include <glbinding/gl/types.h>

#include "../asset.h"
#include "material.h"

// this class is just meant to own the data and buffers. It's NOT responsible for rendering.
// seperate classes are to be setup to do that.

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

	gl::GLuint get_vertex_array_id() const;
	gl::GLuint get_vertex_buffer_id() const;
	gl::GLuint get_index_buffer_id() const;
	gl::GLuint get_index_element_count() const;
	gl::GLint  get_material_index() const;

private:

	gl::GLuint m_vertex_array_id { 0 };
	gl::GLuint m_vertex_buffer_id { 0 };
	gl::GLuint m_index_buffer_id { 0 };
	gl::GLuint m_index_element_count{ 0 };
	gl::GLint  m_material_index { -1 };
};

#endif // _MESH_H_
