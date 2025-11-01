#ifndef _TEXT_UTILITIES_H_
#define _TEXT_UTILITIES_H_

namespace text_utilities
{
	bool is_character_white_space(char32_t character);
	char32_t utf8_to_char32(const char* utf8str);
}

#endif // _TEXT_UTILITIES_H_