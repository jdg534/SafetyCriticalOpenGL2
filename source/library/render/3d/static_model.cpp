#include "static_model.h"

#include <glm/gtc/type_ptr.inl>

#include "../../assets/3d/model.h"
#include "../../assets/texture.h"

// public
/////////

static_model::static_model(std::weak_ptr<model> model)
	: renderable_3d()
	, m_model(model)
{
	set_renderable_type(renderable_type::STATIC_GEOMETRY);
}

static_model::~static_model()
{
	renderable_3d::~renderable_3d();
}

void static_model::initialise()
{
	// place holder.
}

void static_model::shutdown()
{
	// nothing to shutdown, this doesn't own the data.
}

void static_model::draw()
{
	using namespace gl;

	const GLuint shader_id = get_shader_program();

	const GLint u_model_location = glGetUniformLocation(shader_id, "u_model");
	const GLint u_diffuse_map_location = glGetUniformLocation(shader_id, "u_diffuse_map");
	const GLint u_surface_tint_location = glGetUniformLocation(shader_id, "u_surface_tint"); // use the diffuse colour from the material.

	const glm::mat4x4 net_transform = get_net_transform();

	glUniformMatrix4fv(u_model_location, 1, GL_FALSE, glm::value_ptr(net_transform));

	// start of mesh specfic 
	auto model = m_model.lock();

	const auto& materials = model->get_materials();
	for (const auto& mesh : model->get_meshs())
	{
		const auto& material = materials[mesh->get_material_index()];
		const auto& textures = material->get_textures();
		const GLuint vertex_array_id = mesh->get_vertex_array_id();
		const GLuint vertex_buffer_id = mesh->get_vertex_buffer_id();
		const GLuint index_buffer_id = mesh->get_index_buffer_id();
		const GLuint index_element_count = mesh->get_index_element_count();

		glActiveTexture(GL_TEXTURE0);
		if (textures.find(texture_purpose::diffuse_map) != textures.cend())
		{
			glBindTexture(GL_TEXTURE_2D, textures.at(texture_purpose::diffuse_map).lock()->get_id());
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		glUniform1i(u_diffuse_map_location, 0); // 0 as in GL_TEXTURE0

		// u_surface_tint_location 
		glUniform4fv(u_surface_tint_location, 1, glm::value_ptr(material->get_diffuse_colour()));

		// buffers
		glBindBuffer(GL_ARRAY_BUFFER, mesh->get_vertex_buffer_id());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->get_index_buffer_id());
		glBindVertexArray(mesh->get_vertex_array_id());

		// draw
		glDrawElements(GL_TRIANGLES, mesh->get_index_element_count(), GL_UNSIGNED_SHORT, 0);
		glBindVertexArray(0); // clear the vertex array.
	}
}