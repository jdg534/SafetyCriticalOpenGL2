#include "quad.h"

#include <glm/gtc/type_ptr.inl>

using namespace gl;

// public
/////////

quad::quad(const std::weak_ptr<texture>& texture, const glm::vec2& position, const glm::vec2& size)
	: renderable_2d()
	, m_texture(texture)
	, m_tint(1.0f, 1.0f, 1.0f, 1.0f)
{
	set_renderable_type(renderable_type::_2D_GEOMETRY);
	using namespace gl;
	using namespace vertex_types;
	// 0,0 is bottom left in texutre coordinates.

	// patten:
	// 0-1-2
	// 3-2-1

	const glm::vec2 quarer_size = { size.x * 0.25f, size.y * 0.25f };

	vertex_2d vertex_buffer_data[4] = // pos, tc
	{
		{ glm::vec2 { position.x - quarer_size.x, position.y - quarer_size.y },glm::vec2{0.0f, 1.0}},
		{ glm::vec2 { position.x + quarer_size.x, position.y - quarer_size.y },glm::vec2{1.0, 1.0}},
		{ glm::vec2 { position.x - quarer_size.x, position.y + quarer_size.y },glm::vec2{0.0, 0.0}},
		{ glm::vec2 { position.x + quarer_size.x, position.y + quarer_size.y },glm::vec2{1.0, 0.0}}
	};
	constexpr GLsizeiptr vertex_buffer_size = vertex2d_struct_size * 4;
	
	unsigned short index_buffer[6] =
	{
		0,1,2,
		3,2,1
	};

	constexpr GLsizeiptr index_buffer_size = sizeof(unsigned short) * 6;

	GLuint buffer_ids[2];
	glGenBuffers(2, buffer_ids);
	GLuint vertex_array_id;
	glGenVertexArrays(1, &vertex_array_id);

	// bind the vertex array
	glBindVertexArray(vertex_array_id);
	
	glBindBuffer(GL_ARRAY_BUFFER, buffer_ids[0]);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size, vertex_buffer_data, GL_STATIC_DRAW);
	set_vertex_buffer_id(buffer_ids[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_ids[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_size, index_buffer, GL_STATIC_DRAW);
	set_index_buffer_id(buffer_ids[1]);
	set_start_in_index_buffer(0);
	set_index_count(4);

	// layout(location = 0): vec2 position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, vertex2d_struct_size, (void*)offsetof(vertex_2d, position));

	// layout(location = 1): vec2 uv
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, vertex2d_struct_size, (void*)offsetof(vertex_2d, texture_coordinates));
	set_vertex_array_id(vertex_array_id);
	// Unbind VAO (safe cleanup)
	glBindVertexArray(0);
}

quad::~quad()
{

}

void quad::initialise()
{
	// done in the constructor
}

void quad::shutdown()
{
	using namespace gl;
	// delete the buffers 
	GLuint buffer_ids[2]{ get_vertex_buffer_id(), get_index_buffer_id() };
	glDeleteBuffers(2, buffer_ids);
	GLuint vertex_array_id = get_vertex_array_id();
	glDeleteVertexArrays(1, &vertex_array_id);
}

void quad::draw()
{
	const GLint shader_program = get_shader_program();

	// --- Uniforms ---
	GLint loc_transform = glGetUniformLocation(shader_program, "u_transform");
	GLint loc_tint = glGetUniformLocation(shader_program, "u_tint");
	GLint loc_alpha_cut = glGetUniformLocation(shader_program, "u_alpha_cut_off");
	GLint loc_texture = glGetUniformLocation(shader_program, "u_texture");

	glm::mat3 transform_matrix_to_pass = get_transform();

	glUniformMatrix3fv(loc_transform, 1, GL_FALSE, glm::value_ptr(transform_matrix_to_pass));
	glUniform4f(loc_tint, 1.0f, 1.0f, 1.0f, 1.0f);
	glUniform1f(loc_alpha_cut, 0.0f);
	glUniform1i(loc_texture, 0); // 0 as in GL_TEXTURE0

	// --- Texture binding ---
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture.lock()->get_id());


	glBindBuffer(GL_ARRAY_BUFFER, get_vertex_buffer_id());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, get_index_buffer_id());
	glBindVertexArray(get_vertex_array_id()); // drawable specfic
	glDrawElements(GL_TRIANGLES, get_index_count(), GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);
}

glm::vec4 quad::get_tint() const
{
	return m_tint;
}

void quad::set_tint(const glm::vec4& tint)
{
	m_tint = tint;
}

