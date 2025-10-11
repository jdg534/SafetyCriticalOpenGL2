#include "text_block.h"

#include <cassert>
#include "../../vertex_types.h"

text_block::text_block(const std::vector<char32_t>& starting_text, const std::weak_ptr<font>& font_to_use, size_t character_limit)
	: renderable_2d()
	, m_text(starting_text)
	, m_font_to_use(font_to_use)
	, m_character_limit(character_limit)
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
	m_glyphs.clear();
	m_text.clear();
	gl::GLuint buffer_ids[2]{ get_vertex_buffer_id(), get_index_buffer_id() };
	gl::glDeleteBuffers(2, buffer_ids);
	set_vertex_buffer_id(0);
	set_index_buffer_id(0);
}

void text_block::draw()
{

}

void text_block::set_text(const std::vector<char32_t>& new_text)
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

void text_block::setup_glyphs()
{
	assert(m_character_limit > 0);
	assert(m_text.size() <= m_character_limit);
	assert(m_glyphs.size() == 0); // this should only change once
	assert(!m_font_to_use.expired());

	m_glyphs.resize(m_character_limit);

	// make the buffer, want to get to off set.
	gl::GLuint buffer_ids[2];
	gl::glGenBuffers(2, buffer_ids); // [0] vertex buffer, [1] index buffer
	// buffer size vertex2d_struct_size * 4 * character limit
	const size_t vertex_buffer_size = vertex_types::vertex2d_struct_size * m_character_limit * 4;
	std::vector<vertex_types::vertex_2d> vertex_buffer_data;
	vertex_buffer_data.resize(m_character_limit * 4);
	std::vector<unsigned short> index_buffer;
	index_buffer.resize(m_character_limit * 4);
	const size_t index_buffer_size = sizeof(unsigned short) * m_character_limit * 4;
	// set the buffers, we'll override the buffer data in update_glyphs()
	gl::glBindBuffer(gl::GL_ARRAY_BUFFER, buffer_ids[0]);
	gl::glBufferData(gl::GL_ARRAY_BUFFER, vertex_buffer_size, vertex_buffer_data.data(), gl::GL_STATIC_DRAW);
	set_vertex_buffer_id(buffer_ids[0]);
	gl::glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, buffer_ids[1]);
	gl::glBufferData(gl::GL_ELEMENT_ARRAY_BUFFER, index_buffer_size, index_buffer.data(), gl::GL_STATIC_DRAW);
	set_index_buffer_id(buffer_ids[1]);
	set_start_in_index_buffer(0);
	set_index_count(0); // the quads will do this
	for (size_t glyph_index = 0; glyph_index < m_character_limit; ++glyph_index)
	{
		m_glyphs[glyph_index] = std::make_unique<glyph>();
		m_glyphs[glyph_index]->set_parent(weak_from_this());
	}
	update_glyphs();
}

void text_block::update_glyphs()
{
	assert(m_glyphs.size() == m_character_limit);
	// pick up here. set the state.
	const size_t vertex_buffer_size = vertex_types::vertex2d_struct_size * m_character_limit * 4;
	const size_t index_buffer_size = sizeof(unsigned short) * m_character_limit * 4;

	std::vector<vertex_types::vertex_2d> vertex_buffer_data;
	vertex_buffer_data.resize(m_character_limit * 4);
	std::memset(&vertex_buffer_data[0], 0, vertex_buffer_size);
	std::vector<unsigned short> index_buffer;
	index_buffer.resize(m_character_limit * 4);
	std::memset(&vertex_buffer_data[0], 0, vertex_buffer_size);

	float current_x = 0.0f;
	float current_y = 0.0f;

	for (int glyph_index = 0; glyph_index < m_character_limit; ++glyph_index)
	{
		
	}

	gl::glBindBuffer(gl::GL_ARRAY_BUFFER, get_vertex_buffer_id());
	gl::glBufferData(gl::GL_ARRAY_BUFFER, vertex_buffer_size, vertex_buffer_data.data(), gl::GL_STATIC_DRAW);
	gl::glBindBuffer(gl::GL_ELEMENT_ARRAY_BUFFER, get_index_buffer_id());
	gl::glBufferData(gl::GL_ELEMENT_ARRAY_BUFFER, index_buffer_size, index_buffer.data(), gl::GL_STATIC_DRAW);
}
