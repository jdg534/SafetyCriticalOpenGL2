#include "text_utilities.h"

#include <algorithm>
#include <array>
#include <stdexcept>

bool text_utilities::is_character_white_space(char32_t character)
{
	using namespace std;
	constexpr array<char32_t, 5> white_space_characters { ' ','\t','\n', '\r', '\0' };
	return any_of(begin(white_space_characters), end(white_space_characters), [character](char32_t white_space_character) { return character == white_space_character; });
}

char32_t text_utilities::utf8_to_char32(const char* utf8str)
{
	// llm generated... only 1 character length strings should be used.
	const unsigned char* s = reinterpret_cast<const unsigned char*>(utf8str);
	if (*s < 0x80)
	{
		return *s;
	}
	else if ((*s >> 5) == 0x6)
	{
		return ((s[0] & 0x1F) << 6) | (s[1] & 0x3F);
	}
	else if ((*s >> 4) == 0xE)
	{
		return ((s[0] & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
	}
	else if ((*s >> 3) == 0x1E)
	{
		return ((s[0] & 0x07) << 18) | ((s[1] & 0x3F) << 12)
			| ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
	}
	throw std::runtime_error("Invalid UTF-8 sequence");
	return '\0';
}

void text_utilities::append_vec3(std::u32string& append_to, const glm::vec3& to_append)
{
	char tmp[128];

	// Use a safe format — choose precision and format that suits you.
	// "%.4f" is predictable and locale neutral (C locale), produces ASCII digits and '.' for decimal.
	int written = std::snprintf(tmp, sizeof(tmp),
		"(%.4f, %.4f, %.4f)",
		static_cast<double>(to_append.x),
		static_cast<double>(to_append.y),
		static_cast<double>(to_append.z));
	const size_t add = static_cast<size_t>(written);
	const size_t start = append_to.size();
	append_to.resize(start + add); // may allocate once if capacity insufficient

	// copy bytes -> char32_t codepoints
	for (size_t i = 0; i < add; ++i)
	{
		append_to[start + i] = static_cast<char32_t>(static_cast<unsigned char>(tmp[i]));
	}
}
