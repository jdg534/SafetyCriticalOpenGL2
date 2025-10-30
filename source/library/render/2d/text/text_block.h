#ifndef _TEXT_BLOCK_H_
#define _TEXT_BLOCK_H_

#include "../../../assets/font.h"
#include "../renderable_2d.h"
#include "../../vertex_types.h"

#include <memory>
#include <vector>

// concuptually this is a array of glyphs with a root transform.
// An instance has ownership of the relevent buffers.
enum class line_spaceing : uint8_t
{
	FIXED,
	RELATIVE_1_2,
	RELATIVE_1_5,
	DOUBLE
};

class text_block
	: public renderable_2d
	, std::enable_shared_from_this<text_block>
{
public:

	text_block() = delete;
	text_block(const std::u32string& starting_text, const std::weak_ptr<font>& font_to_use, size_t character_limit, line_spaceing line_spaceing);

	void set_text(const std::u32string& new_text);

	void initialise() override;
	void shutdown() override;
	void draw() override;

private:

	void setup_glyphs(); // call once.
	void update_glyphs();
	float get_vertical_spacing_modifier() const;

	const std::uint16_t m_character_limit;
	std::u32string m_text;
	std::weak_ptr<font> m_font_to_use;
	line_spaceing m_line_spaceing;
};



#endif // _TEXT_BLOCK_H_