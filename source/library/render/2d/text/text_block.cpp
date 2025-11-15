#include "text_block.h"

#include "../../vertex_types.h"
#include "../../../assets/texture.h"

#include <cassert>
#include <glm/gtc/type_ptr.inl>

using namespace gl;
using namespace vertex_types;

// public
/////////

text_block::text_block(const std::u32string& starting_text, const std::weak_ptr<font>& font_to_use, size_t character_limit, line_spaceing line_spaceing)
	: renderable_2d({1.0f, 1.0f, 1.0f, 1.0f})
	, m_text(starting_text)
	, m_font_to_use(font_to_use)
	, m_character_limit(character_limit)
	, m_line_spaceing(line_spaceing)
{
	m_text.reserve(character_limit);
}

void text_block::initialise()
{
	if (!m_font_to_use.lock()->is_string_supported(m_text))
	{
		throw std::exception("string not supportable by font used.");
		return;
	}
	setup_glyphs();
}

void text_block::shutdown()
{
	m_text.clear();
	GLuint buffer_ids[2]{ get_vertex_buffer_id(), get_index_buffer_id() };
	glDeleteBuffers(2, buffer_ids);
	GLuint vertex_array_id = get_vertex_array_id();
	glDeleteVertexArrays(1, &vertex_array_id);
	set_vertex_buffer_id(0);
	set_index_buffer_id(0);
	set_vertex_array_id(0);
}

// private
//////////

void text_block::draw()
{
	// set the uniforms, vertex and index buffers. the draw
	// uniforms, as would be present within:
	// excluding non object specfic uniforms like frame_buffer_size.
	const GLint shader_program = get_shader_program();

	const size_t n_glyphs_to_draw = std::min(m_text.size(), static_cast<size_t>(m_character_limit));
	const GLsizei index_count = n_glyphs_to_draw * 6;

	const GLint u_tint_loc = glGetUniformLocation(shader_program, "u_tint");
	const GLint u_alphaCut_loc = glGetUniformLocation(shader_program, "u_alpha_cut_off");
	const GLint u_transform_loc = glGetUniformLocation(shader_program, "u_transform");
	const GLint u_texture_loc = glGetUniformLocation(shader_program, "u_texture");

	const glm::mat4x4 net_transform = get_net_transform();

	glUniform4fv(u_tint_loc, 1, glm::value_ptr(get_tint()));
	glUniform1f(u_alphaCut_loc, 0.0f);
	glUniformMatrix4fv(u_transform_loc, 1, GL_FALSE, glm::value_ptr(net_transform));
	
	// set the texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_font_to_use.lock()->get_texture().lock()->get_id());
	glUniform1i(u_texture_loc, 0); // 0 as in GL_TEXTURE0

	// buffers
	glBindBuffer(GL_ARRAY_BUFFER, get_vertex_buffer_id());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, get_index_buffer_id());
	glBindVertexArray(get_vertex_array_id());

	// draw
	glBindVertexArray(get_vertex_array_id()); // clear the vertex array.
	glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0); // clear the vertex array.
}

void text_block::set_text(const std::u32string& new_text)
{
	if (new_text.size() > m_character_limit)
	{
		throw std::exception("tried to use too many characters.");
		return;
	}
	if (!m_font_to_use.lock()->is_string_supported(new_text))
	{
		throw std::exception("string not supportable by font used.");
		return;
	}
	m_text = new_text;
	update_glyphs();
}

void text_block::set_line_spacing(line_spaceing line_spacing)
{
	m_line_spaceing = line_spacing;
	update_glyphs();
}

line_spaceing text_block::get_line_spacing() const
{
	return m_line_spaceing;
}

std::u32string_view text_block::get_text() const
{
	return m_text;
}

void text_block::setup_glyphs()
{
	assert(m_character_limit > 0);
	assert(m_text.size() <= m_character_limit);
	assert(!m_font_to_use.expired());

	GLuint vertex_array_id = 0;
	glGenVertexArrays(1, &vertex_array_id);
	glBindVertexArray(vertex_array_id);

	// make the buffer, want to get to off set.
	GLuint buffer_ids[2];
	glGenBuffers(2, buffer_ids); // [0] vertex buffer, [1] index buffer
	// buffer size vertex2d_struct_size * 4 * character limit
	const size_t vertex_buffer_size = vertex_types::vertex2d_struct_size * m_character_limit * 4;
	std::vector<vertex_types::vertex_2d> vertex_buffer_data;
	vertex_buffer_data.resize(m_character_limit * 4);
	std::vector<unsigned short> index_buffer;
	index_buffer.resize(m_character_limit * 4);
	const size_t index_buffer_size = sizeof(unsigned short) * m_character_limit * 4;
	// set the buffers, we'll override the buffer data in update_glyphs()
	glBindBuffer(GL_ARRAY_BUFFER, buffer_ids[0]);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size, vertex_buffer_data.data(), GL_STATIC_DRAW);
	set_vertex_buffer_id(buffer_ids[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_ids[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_size, index_buffer.data(), GL_STATIC_DRAW);
	set_index_buffer_id(buffer_ids[1]);
	set_start_in_index_buffer(0);
	set_index_count(m_text.size() * 6);
	
	// layout(location = 0): vec2 position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, vertex2d_struct_size, (void*)offsetof(vertex_2d, position));

	// layout(location = 1): vec2 uv
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, vertex2d_struct_size, (void*)offsetof(vertex_2d, texture_coordinates));
	set_vertex_array_id(vertex_array_id);
	// Unbind VAO (safe cleanup)
	glBindVertexArray(0);
	update_glyphs();
}

void text_block::update_glyphs()
{
	const size_t vertex_buffer_size = vertex2d_struct_size * m_character_limit * 4;
	const size_t index_buffer_size = sizeof(unsigned short) * m_character_limit * 6;

	std::vector<vertex_2d> vertex_buffer_data;
	vertex_buffer_data.resize(m_character_limit * 4);
	std::memset(&vertex_buffer_data[0], 0, vertex_buffer_size);
	std::vector<unsigned short> index_buffer;
	index_buffer.resize(m_character_limit * 6);
	std::memset(&index_buffer[0], 0, index_buffer_size);

	float current_x = 0.0f;
	float current_y = 0.0f;
	
	// remember that memset 0 was done on both buffers (everything will be set to 0)
	// 0,0 is bottom left in texutre coordinates.

	// patten:
	// 0-1-2
	// 3-2-1

	const auto font_ptr = m_font_to_use.lock();
	const auto texture = font_ptr->get_texture();
	const unsigned int atlas_width = texture.lock()->get_width();
	const unsigned int atlas_height = texture.lock()->get_height();
	const float tallest_glyph_height = static_cast<float>(font_ptr->get_character_height());

	const int num_glyphs_to_set = std::min(static_cast<int>(m_character_limit), static_cast<int>(m_text.size()));

	for (int glyph_index = 0; glyph_index < num_glyphs_to_set; ++glyph_index)
	{
		const char32_t previous_glyph = glyph_index == 0 ? m_text[0] : m_text[glyph_index - 1];
		const glyph_info previous_glyph_info = font_ptr->get_glyph_info(previous_glyph);
		const char32_t current_glyph = m_text[glyph_index];
		const glyph_info current_glyph_info = font_ptr->get_glyph_info(current_glyph);
		const float current_glyph_width = current_glyph_info.right_px - current_glyph_info.left_px;
		const float current_glyph_height = current_glyph_info.bottom_px - current_glyph_info.top_px;
		const kerning_info kerning_info = font_ptr->get_kerning_info(previous_glyph, current_glyph);
		const source_rect glyph_texture_coordinates = font_ptr->get_texture_coordinates_for_glyph(current_glyph);
		const int vertex_buffer_index_offset = 4 * glyph_index;
		const int index_buffer_offset = 6 * glyph_index;
		vertex_buffer_data[vertex_buffer_index_offset + 0].position.x = current_x;
		vertex_buffer_data[vertex_buffer_index_offset + 0].position.y = current_y;
		vertex_buffer_data[vertex_buffer_index_offset + 0].texture_coordinates.x = glyph_texture_coordinates.left;
		vertex_buffer_data[vertex_buffer_index_offset + 0].texture_coordinates.y = glyph_texture_coordinates.top;

		vertex_buffer_data[vertex_buffer_index_offset + 1].position.x = current_x + current_glyph_width;
		vertex_buffer_data[vertex_buffer_index_offset + 1].position.y = current_y;
		vertex_buffer_data[vertex_buffer_index_offset + 1].texture_coordinates.x = glyph_texture_coordinates.right;
		vertex_buffer_data[vertex_buffer_index_offset + 1].texture_coordinates.y = glyph_texture_coordinates.top;

		vertex_buffer_data[vertex_buffer_index_offset + 2].position.x = current_x;
		vertex_buffer_data[vertex_buffer_index_offset + 2].position.y = current_y + current_glyph_height;
		vertex_buffer_data[vertex_buffer_index_offset + 2].texture_coordinates.x = glyph_texture_coordinates.left;
		vertex_buffer_data[vertex_buffer_index_offset + 2].texture_coordinates.y = glyph_texture_coordinates.bottom;

		vertex_buffer_data[vertex_buffer_index_offset + 3].position.x = current_x + current_glyph_width;
		vertex_buffer_data[vertex_buffer_index_offset + 3].position.y = current_y + current_glyph_height;
		vertex_buffer_data[vertex_buffer_index_offset + 3].texture_coordinates.x = glyph_texture_coordinates.right;
		vertex_buffer_data[vertex_buffer_index_offset + 3].texture_coordinates.y = glyph_texture_coordinates.bottom;

		// patten:
		// 0-1-2
		// 3-2-1
		index_buffer[index_buffer_offset + 0] = vertex_buffer_index_offset + 0;
		index_buffer[index_buffer_offset + 1] = vertex_buffer_index_offset + 1;
		index_buffer[index_buffer_offset + 2] = vertex_buffer_index_offset + 2;

		index_buffer[index_buffer_offset + 3] = vertex_buffer_index_offset + 3;
		index_buffer[index_buffer_offset + 4] = vertex_buffer_index_offset + 2;
		index_buffer[index_buffer_offset + 5] = vertex_buffer_index_offset + 1;

		// update current_x & current_y.
		current_x += current_glyph_width + kerning_info.additional_spacing;
		if (current_glyph == '\r' || current_glyph == '\n')
		{
			// new line, only dealing with left alighment for now.
			current_x = 0.0f;
			current_y += tallest_glyph_height * get_vertical_spacing_modifier();
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, get_vertex_buffer_id());
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size, vertex_buffer_data.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, get_index_buffer_id());
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_size, index_buffer.data(), GL_STATIC_DRAW);
}

float text_block::get_vertical_spacing_modifier() const
{
	switch (m_line_spaceing)
	{
		case line_spaceing::FIXED: return 1.0f;
		case line_spaceing::RELATIVE_1_2: return 1.2f;
		case line_spaceing::RELATIVE_1_5: return 1.5f;
		case line_spaceing::DOUBLE: return 2.0f;
	}
	return 1.0f;
}
