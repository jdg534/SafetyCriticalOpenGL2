#ifndef _TEXT_BLOCK_H_
#define _TEXT_BLOCK_H_

#include "../../../assets/font.h"
#include "../renderable_2d.h"
#include "../../vertex_types.h"
#include "glyph.h"

#include <memory>
#include <vector>

// concuptually this is a array of glyphs, but this does define a root transform.
// also ownership of the relevent buffers.

class text_block
	: public renderable_2d
	, std::enable_shared_from_this<text_block>
{
public:

	text_block() = delete;
	text_block(const std::u32string& starting_text, const std::weak_ptr<font>& font_to_use, size_t character_limit);

	void set_text(const std::u32string& new_text);

	void initialise() override;
	void shutdown() override;
	void draw() override;

private:

	void setup_glyphs(); // call once.
	void update_glyphs();

	const std::uint16_t m_character_limit;
	std::u32string m_text;
	std::vector<std::unique_ptr<glyph>> m_glyphs;
	std::weak_ptr<font> m_font_to_use;
};



#endif // _TEXT_BLOCK_H_