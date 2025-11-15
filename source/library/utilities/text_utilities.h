#ifndef _TEXT_UTILITIES_H_
#define _TEXT_UTILITIES_H_

#include <string>
#include <glm/glm.hpp>

namespace text_utilities
{
	bool is_character_white_space(char32_t character);
	char32_t utf8_to_char32(const char* utf8str);
	void append_vec3(std::u32string& append_to, const glm::vec3& to_append);

}

#endif // _TEXT_UTILITIES_H_