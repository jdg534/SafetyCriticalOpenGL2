#include "mesh.h"

#include <string_view>
#include <filesystem>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "../../render/vertex_types.h"
#include "../../render/include_opengl.h" // not solid correct.

using namespace std;
using namespace gl;

// public
/////////

mesh::mesh(const std::string& name, const std::string& path, std::weak_ptr<const asset_manager> asset_manager)
	: asset(name, path, asset_manager)
{

}

mesh::~mesh() { }

void mesh::initialise()
{
    // this is a placeholder, initialise_assimp_struct() should be called instead.
}

void mesh::initialise_assimp_struct(const aiMesh* initialise_with)
{
    using namespace vertex_types;

    if (initialise_with->HasBones())
    {
        throw runtime_error("Attempted to load rigged mesh, this isn't currently supported"); // if there's demand 
    }
    
    if (!initialise_with->HasTextureCoords(0) || !initialise_with->HasPositions() || !initialise_with->HasNormals())
    {
        throw runtime_error("Only meshes with normals and texture coordinates are supported");
    }

    std::vector<vertex_3d> vertex_buffer_data;
    std::vector<unsigned short> index_buffer_data;

    vertex_buffer_data.resize(initialise_with->mNumVertices);
    index_buffer_data.resize(initialise_with->mNumFaces * 3);

    const GLsizeiptr vertex_buffer_size = vertex_buffer_data.size() * vertex3d_struct_size;
    const GLsizeiptr index_buffer_size = index_buffer_data.size() * sizeof(unsigned short);

    for (unsigned int i = 0; i < initialise_with->mNumVertices; ++i)
    {
        vertex_buffer_data[i].position.x = initialise_with->mVertices[i].x;
        vertex_buffer_data[i].position.y = initialise_with->mVertices[i].y;
        vertex_buffer_data[i].position.z = initialise_with->mVertices[i].z;

        vertex_buffer_data[i].normal.x = initialise_with->mNormals[i].x;
        vertex_buffer_data[i].normal.y = initialise_with->mNormals[i].y;
        vertex_buffer_data[i].normal.z = initialise_with->mNormals[i].z;

        vertex_buffer_data[i].texture_coordinates.x = initialise_with->mTextureCoords[0][i].x;
        vertex_buffer_data[i].texture_coordinates.y = initialise_with->mTextureCoords[0][i].y;
    }

    for (unsigned int i = 0; i < initialise_with->mNumFaces; ++i)
    {
        const aiFace& face = initialise_with->mFaces[i];
        if (face.mNumIndices != 3)
        {
            throw runtime_error("Only triangular meshes are supported!");
        }
        const unsigned int write_index = i * 3;
        index_buffer_data[write_index + 0] = face.mIndices[0];
        index_buffer_data[write_index + 1] = face.mIndices[1];
        index_buffer_data[write_index + 2] = face.mIndices[2];
    }

    glGenVertexArrays(1, &m_vertex_array_id);
    glBindVertexArray(m_vertex_array_id);

    // make the buffer, want to get to off set.
    GLuint buffer_ids[2];
    glGenBuffers(2, buffer_ids); // [0] vertex buffer, [1] index buffer

    glBindBuffer(GL_ARRAY_BUFFER, buffer_ids[0]);
    glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size, vertex_buffer_data.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_ids[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_size, index_buffer_data.data(), GL_STATIC_DRAW);

    // layout(location = 0): vec3 position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertex3d_struct_size, (void*)offsetof(vertex_3d, position));

    // layout(location = 1): vec2 uv
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, vertex3d_struct_size, (void*)offsetof(vertex_3d, texture_coordinates));

    // layout(location = 2) in vec3 normal;
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_TRUE, vertex3d_struct_size, (void*)offsetof(vertex_3d, normal));

    m_vertex_buffer_id = buffer_ids[0];
    m_index_buffer_id = buffer_ids[1];
    m_index_element_count = index_buffer_data.size();
    m_material_index = initialise_with->mMaterialIndex;

    // Unbind VAO (safe cleanup)
    glBindVertexArray(0);
}

void mesh::shutdown()
{
    GLuint buffer_ids[2]{ m_vertex_buffer_id, m_index_buffer_id };
    glDeleteBuffers(2, buffer_ids);
    glDeleteVertexArrays(1, &m_vertex_array_id);
    m_vertex_array_id = m_vertex_buffer_id = m_index_buffer_id = m_index_element_count = 0;
}

asset_type mesh::get_type() const
{
	return asset_type::mesh;
}

gl::GLuint mesh::get_vertex_array_id() const { return m_vertex_array_id; }
gl::GLuint mesh::get_vertex_buffer_id() const { return m_vertex_buffer_id; }
gl::GLuint mesh::get_index_buffer_id() const { return m_index_buffer_id; }
gl::GLuint mesh::get_index_element_count() const { return m_index_element_count; }
gl::GLint  mesh::get_material_index() const { return m_material_index; }

// private
//////////
